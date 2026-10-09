// LevelUpUi.h - M4 character-screen UI of Level-Up Override 1.0.0: Configure button (R6) with the R9 nudge, the single options page (OptionsLogic.h; WizardLogic.h supplies the Item type),
// the R11 status line. NOT a standalone header: LevelUpOverride.cpp includes it INSIDE its anonymous namespace after the gates (it uses the .cpp's
// tagConfig / evalStats / ptApply / ptRemove / g_presets / bankedLevels / logLine), and includes GuiKit.h + WizardLogic.h at file scope.
//
// Lifecycle per character-screen panel (CSWGuiInGameCharacter):
//   ctor hook 0x84D82C -> uiPanelCtor(panel): the layout GFF is still open, so the labels are created here from templates (addToList=0) and
//     stored in a record keyed by the panel pointer (4 records, round robin). Nothing is appended yet (Mod 14 binds its ids 69..150 later).
//   SetStats hook 0x84F221 (every frame while the screen is the updated panel) -> uiTick(panel, stats, realCanLevelUp, heldResult): the first
//     tick appends every control in z-order, then each tick places (scale from BTN_LEVELUP = control index 66), shows/hides, sets text, polls clicks.
// Batch 3a: the Configure button shows only while the engine shows BTN_LEVELUP/BTN_AUTO (heldResult != 0); otherwise a small icon button (C_ICON,
//   top-right corner of the 3D view, research/companion_levelup/r10/ui_icon) opens the same page. The PC (empty tag) gets the R10 filter page
//   (R10Page.h) in the same controls (bg, title, rows, Apply, Cancel), saved as the playthrough's `pc` line (ptSetPc).
// Templates: plain LBL_EXPERIENCE_STAT (green text, unit rect 60,449,108,18), framed LBL_XP_BACK (framed fill, 54,431,223,39).
// Batch 3c: the icon is the embedded gear (EmbedTex.h + LuoTextures.inc; gold luo_gear_hi on hover), square in engine px (uiIconRect); the page
//   back is ONE opaque framed window from Mod 14's LBL_EFX_BACK when its layout has it (r->win), else the three stacked LBL_XP_BACK clones.
// Batch 3d: the gear is its own plain label (C_GEAR, uiGearRect) on top of a normal framed C_ICON button; hovered buttons take vanilla's HILIGHT
//   COLOR (border +0x28, uiHoverTint); the icon stays shown on an open page and closes it; BG0 gets the opaque embedded luo_fill (r->oneBack).
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
enum UiCtl { C_CFG, C_STATUS, C_BG0, C_BG1, C_BG2, C_TITLE, C_ROW0, C_BUILD = C_ROW0 + NumRows, C_INSTANT, C_BANK, C_PAUSE, C_PMINUS, C_PPLUS, C_REC, C_APPLY, C_VANILLA, C_CANCEL, C_ICON, C_GEAR, C_TIPBG, C_TIP, C_N };
constexpr int NumBg = 3;
static_assert(NumRows == r10p::RowCount, "the R10 page reuses the option rows one to one");
struct UiSpec { bool framed; int x, y, w, h; };   // layout units (800x600)
// Options page over the stats column only (live 2026-10-02: BTN_AUTO/BTN_LEVELUP, SLD_ALIGN x 310 and LBL_LIGHT/DARK x 304 draw ON TOP of anything
// placed in the 3D-character column, whatever the list order) -> x 54..300, y 134..470. Three stacked framed labels with the black fill = opaque.
constexpr int kRowY0 = 162, kRowStep = 28;   // option rows: y = kRowY0 + kRowStep * (index among the SHOWN rows)
const UiSpec kUiSpec[C_N] = {
    {true, 362, 372, 94, 26},                                                                            // cfgBtn, under BTN_LEVELUP [362,325,94,40]
    {false, 54, 474, 350, 20},                                                                           // status, below LBL_XP_BACK (bottom 470), left of LBL_BAR5 (x 407)
    {true, 54, 134, 246, 336}, {true, 54, 134, 246, 336}, {true, 54, 134, 246, 336},                     // backgrounds
    {false, 62, 138, 230, 20},                                                                           // title
    {true, 62, kRowY0, 230, 24}, {true, 62, kRowY0, 230, 24}, {true, 62, kRowY0, 230, 24},               // option rows (y recomputed per slot)
    {true, 62, kRowY0, 230, 24}, {true, 62, kRowY0, 230, 24}, {true, 62, kRowY0, 230, 24},
    {false, 62, 332, 230, 20},                                                                           // build line
    {true, 62, 356, 113, 26}, {true, 179, 356, 113, 26},                                                 // Instant, Bank for Jedi (plain On/Off toggle)
    {false, 62, 386, 150, 24}, {true, 216, 386, 36, 24}, {true, 256, 386, 36, 24},                       // Pause at level N (label), "-", "+"
    {true, 62, 414, 230, 24},                                                                            // Use recommended
    {true, 62, 442, 72, 26}, {true, 138, 442, 72, 26}, {true, 214, 442, 78, 26},                         // Apply, Vanilla, Cancel
    {true, 492, 172, 28, 28},                                                                            // settings icon: top-right of LBL_3DCHAR [295,134,230,336] (r10/ui_icon README)
    {false, 492, 172, 28, 28},                                                                           // gear glyph on the icon button (placed by uiGearRect)
    {true, 54, 134, 120, 40}, {false, 60, 139, 108, 30}};                                                // tooltip box + text (CUSTOM fallback, placed per hover by uiTipTick; created last = on top)
const char* const kUiName[C_N] = {"cfg", "status", "bg0", "bg1", "bg2", "title", "row0", "row1", "row2", "row3", "row4", "row5", "build", "instant", "bank", "pause", "pminus", "pplus", "recommended", "apply", "vanilla", "cancel", "icon", "gear", "tipbg", "tip"};
const char* const kLossyText = "levelup_playthroughs.ini too large - edit by hand";   // F6: shown instead of the generic failure / status text when the file parse was lossy
const char* const kTplPlain = "LBL_EXPERIENCE_STAT";
const char* const kTplFramed = "LBL_XP_BACK";
const char* const kTplWindow = "LBL_EFX_BACK";   // Mod 14 (live character_p.gui override): opaque frame uibit_brdr_16wc/16we, fill uibit_fill_2bt, FILLSTYLE 0
const char* const kIconFallback = "lbl_icn_opt2";   // vanilla icon used when the embedded gear (or the ui.icon= resref) does not load
constexpr int kIconRight = 520, kIconTop = 172, kIconSide = 28;   // = kUiSpec[C_ICON]: right edge, top, side in layout units (the side follows sy)
constexpr float kGearInset = 0.18f;   // C_GEAR = the icon rect inset by round(side * kGearInset) per side (the button frame shows around it)
const char* const kPageFill = "luo_fill";   // batch 3d: embedded opaque near-black (LuoTextures.inc), stretched over BG0
const float kHiTint[3] = {0.8f, 0.8f, 0.69804f};   // vanilla BTN_LEVELUP HILIGHT COLOR (character_p.gui); its BORDER COLOR is 0.05098, 0.34902, 0.27059
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
    bool pc = false;             // batch 3a: the shown creature is the PC (empty tag): Configure / icon open the R10 filter page
    bool tipPc = false;          // kind the native tip strings are currently set for (uiTipKind)
    int lvlLogged = -1;          // last logged level-up-buttons-shown state (UI lvlbtn line)
    bool iconTried = false, iconOk = false;
    void* iconIm = nullptr;      // the icon's normal fill; iconHiIm = the hover fill (embedded gear only, else null = no swap)
    void* iconHiIm = nullptr;
    bool iconHot = false;        // the hover fill is the one set
    bool win = false;            // batch 3c: C_BG0 was created from kTplWindow (one framed window; BG1/BG2 stay hidden)
    bool fillOk = false;         // batch 3d: BG0 carries the opaque luo_fill
    bool oneBack = false;        // win || fillOk: BG0 alone is the page back
    bool gearOk = false;         // C_GEAR carries the embedded gear (else C_ICON has the fallback fill and C_GEAR stays hidden)
    float baseTint[C_N][3] = {}; // border COLOR read once in uiAppend (tintMask bit i = valid); restored when the hover leaves
    uint32_t tintMask = 0;
    int tinted = -1;             // control currently showing kHiTint
    int lvlPending = -1;         // UI lvlbtn line due on the next tick (the engine writes BTN_LEVELUP's bit1 after our hook)
    // options page
    bool open = false;
    char pageTag[32] = {};
    bool pcPage = false;         // the open page is the PC's R10 filter page (r10 below), not a companion options page
    r10p::State r10 = {};
    filewatch::Stamp pageTable;
    wiz::Item items[wiz::MaxItems] = {};
    int itemPreset[wiz::MaxItems] = {};
    int nItems = 0, recItem = -1;
    opt::State os = {};
    bool instant = false, bank = true, failed = false;
    int curLevel = -1;           // total character level of the page's creature (-1 = unreadable); refreshed by uiTick before input, so clicks can clamp without engine calls
    int pause = 0;               // 0.3.1: bank on + pause N = last character level taken before banking; 0 = preset default (convertAt-1)
    int rowSlot[NumRows] = {};   // compact slot of each shown option row, -1 = hidden
    // hover tooltips (GuiTip.h): native strings are set once in uiAppend; the custom box is the fallback
    guitip::State tip;
    guitip::ProbeSnap tipProbe;
    int tipProbeKey = -1;
    bool tipOpen = false;        // page-open state seen by the previous uiTipTick (a change resets the tip)
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
        uintptr_t c = 0;
        if (i == C_BG0 && (c = guikit::createLabelFromTemplate(panel, kTplWindow)) != 0) r->win = true;   // Mod 14's opaque window, when its layout has it
        if (!c) c = guikit::createLabelFromTemplate(panel, framed ? kTplFramed : kTplPlain);
        if (!c) { usedFramed = !framed; c = guikit::createLabelFromTemplate(panel, framed ? kTplPlain : kTplFramed); }   // missing tag: use the other template
        r->c[i] = c;
        if (!c) continue;
        ++r->nCreated;
        int x, y, w, h;
        if (!r->fb.ok && !(i == C_BG0 && r->win) && guikit::getExtent(c, &x, &y, &w, &h) && w > 0 && h > 0) {   // (the window template's rect is not ours)   // ctor-time extent = the template's rect in engine px
            const int* u = usedFramed ? kFramedRect : kPlainRect;
            r->fb = {static_cast<float>(w) / u[2], static_cast<float>(h) / u[3], static_cast<float>(x), static_cast<float>(y), u[0], u[1], true};
        }
    }
    logLine("UI ctor panel=%08x created=%d/%d fallbackScale=%d", (unsigned)panel, r->nCreated, (int)C_N, r->fb.ok);
    logLine("UI page window=%s", r->win ? "efx" : "stack");
}


// ---- tooltip text (pure; every line <= TipMaxChars chars, <= TipMaxLines lines: the engine box does not wrap, ours is sized from the text) ----
constexpr int TipMaxChars = 36, TipMaxLines = 3;
// nullptr = no tip (status, title, build line, backgrounds, the tip controls themselves). pc = the PC's R10 page texts (batch 3a).
const char* uiTipTextPc(int ctl) {
    switch (ctl) {
    case C_CFG: return "Set your level-up filter: grey\nout feats and powers that do not\nfit your build.";
    case C_ROW0 + r10p::RowOn: return "On: feats and powers that do not\nfit your build are greyed out\nwhen you level up.";
    case C_ROW0 + r10p::RowRole: return "Click to cycle your build's role.";
    case C_ROW0 + r10p::RowStyle: return "Click to cycle your weapon style.";
    case C_ROW0 + r10p::RowAlign: return "Click to cycle your alignment.";
    case C_ROW0 + r10p::RowFeats: return "Feats panel: all = no filter,\nbuild_dim = grey off-build feats,\ncombat / force = grey the rest.";
    case C_ROW0 + r10p::RowPowers: return "Powers panel: all = no filter,\nbuild_dim = grey off-build powers,\ncombat / force = grey the rest.";
    case C_APPLY: return "Save the filter for the current\nplaythrough.";
    default: return nullptr;
    }
}
const char* uiTipText(int ctl, bool pc = false) {
    if (pc && uiTipTextPc(ctl)) return uiTipTextPc(ctl);
    switch (ctl) {
    case C_ICON: return "Level-up settings";
    case C_CFG: return "Choose a level-up plan for this\ncompanion. Unplanned companions\nlevel up as in the vanilla game.";
    case C_ROW0 + 0: return "When (or whether) this companion\nbecomes a Jedi.";
    case C_ROW0 + 1: return "Click to cycle Role choices;\nthe build below updates.";
    case C_ROW0 + 2: return "Click to cycle Weapons choices;\nthe build below updates.";
    case C_ROW0 + 3: return "Click to cycle Powers choices;\nthe build below updates.";
    case C_ROW0 + 4: return "Click to cycle Skills choices;\nthe build below updates.";
    case C_ROW0 + 5: return "Click to cycle Variant choices;\nthe build below updates.";
    case C_INSTANT: return "On: levels apply automatically\nwhen earned. Off: you level up\nby hand, following the plan.";
    case C_BANK: return "On: keep earned levels unspent\nuntil the Jedi class is available,\nthen spend them as Jedi.";
    case C_PAUSE: case C_PMINUS: case C_PPLUS: return "Stay at this level, bank later\nlevels for Jedi. Starts at the\ncurrent level; use + to go higher.";
    case C_REC: return "Switch to the recommended build\nfor this companion.";
    case C_APPLY: return "Save this plan for the current\nplaythrough.";
    case C_VANILLA: return "Remove the plan; this companion\nlevels up as in the vanilla game.";
    case C_CANCEL: return "Close without changes.";
    default: return nullptr;
    }
}

// Controls that take the hover highlight = everything uiClickable can return.
bool uiTintable(int i) {
    return i == C_CFG || i == C_ICON || (i >= C_ROW0 && i < C_ROW0 + NumRows) || i == C_INSTANT || i == C_BANK || i == C_PMINUS || i == C_PPLUS || i == C_REC || i == C_APPLY ||
           i == C_VANILLA || i == C_CANCEL;
}

// First tick: append every control (draw order = append order: the three backgrounds first), set alignments, log once.
void uiAppend(UiPanel* r) {
    int ok = 0;
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i]) continue;
        if (guikit::appendToPanel(r->panel, r->c[i])) ++ok; else r->c[i] = 0;   // a failed control stays hidden and unused
    }
    r->appended = true;
    if (r->win && r->c[C_BG0]) logLine("UI page window back alpha ok=%d", guikit::setAlpha(r->c[C_BG0], 1.0f));   // keeps its own frame / fill / FILLSTYLE
    embtex::g_log = logLine;
    const bool texOk = embtex::ensure(kLuoTex, static_cast<int>(kLuoTexCount));   // gear, gear_hi, page fill (idempotent across panels)
    {   // batch 3d: one copy of a framed back is see-through (its tiled fill is not opaque): stretch the opaque luo_fill over BG0; frame + COLOR stay
        bool st = false;
        void* fim = texOk && r->c[C_BG0] ? guikit::imageFor(kPageFill) : nullptr;
        r->fillOk = fim && guikit::setFill(r->c[C_BG0], fim);
        if (r->fillOk) { st = guikit::setFillStyle(r->c[C_BG0], 2); guikit::setAlpha(r->c[C_BG0], 1.0f); }
        r->oneBack = r->win || r->fillOk;
        logLine("UI page fill=%s ok=%d style=%d one=%d", kPageFill, r->fillOk, st, r->oneBack);
    }
    for (int b = 0; b < NumBg; ++b) {   // opaque background: swap the framed template's translucent fill for black, full alpha (3 stacked = opaque)
        if (!r->c[C_BG0 + b] || r->oneBack) continue;
        const bool f = guikit::setFill(r->c[C_BG0 + b], guikit::imageFor(kPageBackFill));
        const bool a = guikit::setAlpha(r->c[C_BG0 + b], 1.0f);
        logLine("UI page back%d fill=%s ok=%d alpha ok=%d", b, kPageBackFill, f, a);
    }
    if (r->c[C_TIPBG]) {   // tooltip box: same opaque black fill as the page back (one label, alpha 1)
        const bool f = guikit::setFill(r->c[C_TIPBG], guikit::imageFor(kPageBackFill));
        const bool a = guikit::setAlpha(r->c[C_TIPBG], 1.0f);
        logLine("UI tip back fill=%s ok=%d alpha ok=%d", kPageBackFill, f, a);
    }
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i] || (i >= C_BG0 && i <= C_BG2) || i == C_STATUS || i == C_TIPBG) continue;
        guikit::setAlign(r->c[i], i == C_TITLE ? 17 : (i == C_TIP ? 9 : 18));   // 17 top-centre, 18 centre, 9 top-left (multi-line tip); the status line keeps the template's left alignment
    }
    {   // native tooltip strings: static per control, set once here (never from uiClick, which runs inside the engine's SetStats)
        int nat = 0;
        for (int i = 0; i < C_N; ++i) if (r->c[i] && uiTipText(i) && guitip::setNative(r->c[i], uiTipText(i))) ++nat;
        logLine("UI tips native=%d", nat);
    }
    if (r->c[C_ICON]) {   // settings icon texture (ui.icon=, default the embedded gear); then kIconFallback; the text "LU" stays as the last fallback
        const char* name = g_cfg.uiIcon;
        const bool emb = !_stricmp(name, kLuoTex[0].resref);
        bool embOk = emb && texOk;
        if (emb && !embOk) name = kIconFallback;
        void* im = guikit::imageFor(name);
        if (!im && strcmp(name, kIconFallback)) { name = kIconFallback; embOk = false; im = guikit::imageFor(name); }
        // batch 3d: the embedded gear goes on its own label over the framed button (C_ICON keeps the template fill, like the rows); a fallback
        // image replaces C_ICON's fill as before
        const uintptr_t gc = embOk && r->c[C_GEAR] ? r->c[C_GEAR] : r->c[C_ICON];
        r->iconOk = im && guikit::setFill(gc, im) && guikit::setAlpha(gc, 1.0f);
        r->gearOk = r->iconOk && gc == r->c[C_GEAR];
        r->iconIm = im;
        r->iconHiIm = r->iconOk && embOk ? guikit::imageFor(kLuoTex[1].resref) : nullptr;
        const bool st = r->iconOk && guikit::setFillStyle(gc, 2);   // stretch the image over the (square) control
        if (r->gearOk) { guikit::setClickable(gc, false); guikit::setText(gc, ""); }   // clicks and hover go to the button underneath
        r->iconTried = true;
        logLine("UI icon fill=%s ok=%d", name, r->iconOk);
        logLine("UI icon emb=%d hi=%d style=%d", embOk, r->iconHiIm != nullptr, st);
    }
    {   // batch 3d: base border COLOR of every control that can take the hover highlight (restored when the hover leaves)
        int n = 0, got = 0;
        for (int i = 0; i < C_N; ++i) {
            if (!r->c[i] || !uiTintable(i)) continue;
            ++n;
            float v[3];
            if (!guikit::getBorderTint(r->c[i], v) || !(v[0] >= 0.f && v[0] <= 2.f && v[1] >= 0.f && v[1] <= 2.f && v[2] >= 0.f && v[2] <= 2.f)) continue;
            memcpy(r->baseTint[i], v, sizeof v);
            r->tintMask |= 1u << i;
            ++got;
        }
        logLine("UI hover tint base_ok=%d n=%d/%d", n > 0 && got == n, got, n);
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
    for (int row = 0; row < NumRows; ++row) r->rowSlot[row] = (r->open && (r->pcPage || opt::shown(r->os, row))) ? k++ : -1;
}
// Settings icon rect in engine px: square on any screen (sx != sy on a wide one): side = kIconSide units of HEIGHT, right edge pinned at unit
// x kIconRight, top at unit y kIconTop.
void uiIconRect(const guikit::Scale& s, int* x, int* y, int* w, int* h) {
    const int side = static_cast<int>(lroundf(kIconSide * s.sy)), right = guikit::unitX(s, static_cast<float>(kIconRight));
    if (x) *x = right - side;
    if (y) *y = guikit::unitY(s, static_cast<float>(kIconTop));
    if (w) *w = side;
    if (h) *h = side;
}
// Gear glyph rect: the icon rect inset by round(side * kGearInset) per side (still square), so the button frame shows around it.
void uiGearRect(const guikit::Scale& s, int* x, int* y, int* w, int* h) {
    int ix, iy, side;
    uiIconRect(s, &ix, &iy, &side, nullptr);
    const int in = static_cast<int>(lroundf(side * kGearInset));
    if (x) *x = ix + in;
    if (y) *y = iy + in;
    if (w) *w = side - 2 * in;
    if (h) *h = side - 2 * in;
}
void uiPlace(UiPanel* r, const guikit::Scale& s) {
    for (int i = 0; i < C_TIPBG; ++i) {   // the tooltip controls are placed per hover by uiTipTick
        if (!r->c[i]) continue;
        int y = kUiSpec[i].y;
        if (i >= C_ROW0 && i < C_ROW0 + NumRows) y = kRowY0 + kRowStep * (r->rowSlot[i - C_ROW0] >= 0 ? r->rowSlot[i - C_ROW0] : 0);
        int x, py, w, h, ex, ey, ew, eh;
        if (i == C_ICON) uiIconRect(s, &x, &py, &w, &h);
        else if (i == C_GEAR) uiGearRect(s, &x, &py, &w, &h);
        else guikit::unitRect(s, kUiSpec[i].x, y, kUiSpec[i].w, kUiSpec[i].h, &x, &py, &w, &h);
        if (!guikit::getExtent(r->c[i], &ex, &ey, &ew, &eh) || ex != x || ey != py || ew != w || eh != h) guikit::setRect(r->c[i], x, py, w, h);
    }
}
// Hover fill of the gear (only on a change; no-op without a hover image). Batch 3d: on C_GEAR when the gear has its own label.
void uiIconHover(UiPanel* r, bool hot) {
    const uintptr_t g = r->c[r->gearOk ? C_GEAR : C_ICON];
    if (!r->iconHiIm || !g || hot == r->iconHot) return;
    if (guikit::setFill(g, hot ? r->iconHiIm : r->iconIm)) r->iconHot = hot;
}
// Batch 3d: vanilla HILIGHT = the same border images with another COLOR. hov = hovered clickable control (-1 none); only on a change.
void uiHoverTint(UiPanel* r, int hov) {
    if (hov == r->tinted) return;
    if (r->tinted >= 0 && r->c[r->tinted]) guikit::setBorderTint(r->c[r->tinted], r->baseTint[r->tinted]);
    r->tinted = -1;
    if (hov >= 0 && (r->tintMask & (1u << hov)) && r->c[hov] && guikit::setBorderTint(r->c[hov], kHiTint)) r->tinted = hov;
}

// ---- small setters ------------------------------------------------------------------------------------------------------
void uiShow(UiPanel* r, int i, bool vis) {
    if (!vis && i == r->tinted) uiHoverTint(r, -1);   // a hidden control never keeps the highlight
    if (vis) r->shownMask |= 1u << i; else r->shownMask &= ~(1u << i);
    if (r->c[i]) guikit::setVisible(r->c[i], vis);
}
// Page back: one opaque label (r->oneBack: the efx window and/or the luo_fill) or the three stacked labels.
void uiShowBack(UiPanel* r, bool vis) {
    for (int b = 0; b < NumBg; ++b) uiShow(r, C_BG0 + b, vis && (b == 0 || !r->oneBack));
}
void uiSetText(UiPanel* r, int i, const char* t) {
    if (!r->c[i] || !strcmp(r->text[i], t)) return;
    if (guikit::setText(r->c[i], t)) { snprintf(r->text[i], UiText, "%s", t); return; }
    if (!(r->textFail & (1u << i))) { r->textFail |= 1u << i; logLine("UI setText failed ctl=%s panel=%08x (text object not ready?)", kUiName[i], (unsigned)r->panel); }
}
// Settings icon button + its gear glyph (C_GEAR only when it carries the gear).
void uiShowIcon(UiPanel* r, bool vis) {
    uiSetText(r, C_ICON, r->iconOk ? "" : "LU");
    uiShow(r, C_ICON, vis);
    uiShow(r, C_GEAR, vis && r->gearOk);
    if (vis && r->gearOk) guikit::setClickable(r->c[C_GEAR], false);   // setVisible clears click-through: hover / native tip stay on the button
}
// R9 highlight on the Configure button. The base colour is read back once (silently); colour writes happen only when that readback looks sane (Mod 14 1140-1150).
void uiNudge(UiPanel* r, bool on) {
    if (!r->c[C_CFG]) return;
    if (!r->colorTried) {
        r->colorTried = true;
        float v[3];
        r->colorOk = guikit::getTextColor(r->c[C_CFG], v) && v[0] >= 0.f && v[0] <= 2.f && v[1] >= 0.f && v[1] <= 2.f && v[2] >= 0.f && v[2] <= 2.f;
        if (r->colorOk) memcpy(r->base, v, sizeof v);
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
    r->pcPage = false;
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
// 0.3.1 Pause at level N. Only presets with a hold class and a conversion level can pause. Default pause = convertAt-1 (the preset's own rule, stored as 0).
// Explicit range lo .. convertAt-2: lo = max(minPause, the creature's current character level), minPause = max(1, char level of the first present record - 1).
// Range helpers are pure (no engine access): cur = total character level, -1 = unknown (-> lo = minPause).
bool uiCanPause(const Preset& p) { return p.holdClass != 0 && p.convertAt > 0; }
int uiMinPause(const Preset& p) {
    for (int cl = 0; cl <= MaxLevel; ++cl) if (p.lv[cl].present) return cl - 1 > 1 ? cl - 1 : 1;
    return 1;
}
int uiPauseLo(const Preset& p, int cur) { const int mn = uiMinPause(p); return cur > mn ? cur : mn; }
int uiPauseHi(const Preset& p) { return p.convertAt - 1; }   // = the default (stored as 0); an explicit pause is lo .. hi-1
// Pause value as stored: out of range (incl. the default convertAt-1 and above) or not pausable -> 0.
int uiClampPause(const Preset& p, int pause, int cur = -1) {
    if (!uiCanPause(p) || pause < uiPauseLo(p, cur) || pause >= uiPauseHi(p)) return 0;
    return pause;
}
// Effective pause level shown / used: the explicit one, else the default hi.
int uiPauseShown(const Preset& p, int pause) { return pause > 0 ? pause : uiPauseHi(p); }
// Page pause after the preset or creature changed: 0 stays 0 (default); an explicit pause below lo is raised to lo, at/above hi it becomes the default.
int uiNormPause(const Preset& p, int pause, int cur) {
    if (!uiCanPause(p) || pause <= 0) return 0;
    const int lo = uiPauseLo(p, cur);
    if (lo >= uiPauseHi(p)) return 0;
    return pause < lo ? lo : uiClampPause(p, pause, cur);
}
// Pause a bank toggled On (or an unconfigured page) starts with: the creature's current level when it is known and below the default, else 0.
int uiStartPause(const Preset& p, int cur) {
    if (!uiCanPause(p) || cur < 0) return 0;
    const int lo = uiPauseLo(p, cur);
    return lo < uiPauseHi(p) ? lo : 0;
}
bool uiPauseMinusShown(const Preset& p, int pause, int cur) { return uiCanPause(p) && uiPauseShown(p, pause) > uiPauseLo(p, cur); }
bool uiPausePlusShown(const Preset& p, int pause) { return uiCanPause(p) && uiPauseShown(p, pause) < uiPauseHi(p); }
// "-": one level lower, stops at lo. "+": one level higher; reaching hi stores 0 (the default).
void uiPauseStep(const Preset& p, int* pause, int dir, int cur) {
    if (!uiCanPause(p)) { *pause = 0; return; }
    const int lo = uiPauseLo(p, cur), hi = uiPauseHi(p), e = uiPauseShown(p, *pause);
    if (dir < 0) { if (e > lo) *pause = e - 1 >= hi ? 0 : e - 1; }
    else if (e < hi) *pause = e + 1 >= hi ? 0 : e + 1;
}
// Bank toggle: Off -> On starts at the current level (uiStartPause); On -> Off clears the pause. Presets without a pause range just toggle.
void uiBankToggle(const Preset& p, bool* bank, int* pause, int cur) {
    if (!uiCanPause(p)) { *bank = !*bank; *pause = 0; return; }
    if (!*bank) { *bank = true; *pause = uiStartPause(p, cur); }
    else { *bank = false; *pause = 0; }
}
// Text of the Bank control (<= 18 chars: the control is 113 wide).
const char* uiBankText(bool bank, char* buf, size_t cap) { snprintf(buf, cap, "%s", bank ? "Bank for Jedi: On" : "Bank for Jedi: Off"); return buf; }
const char* uiPauseText(const Preset& p, int pause, char* buf, size_t cap) { snprintf(buf, cap, "Pause at level %d", uiPauseShown(p, pause)); return buf; }
// Re-normalises the page's pause into the current preset's range (after a row cycle / Use recommended changed the preset).
void uiClampPagePause(UiPanel* r) { if (r->os.n > 0) r->pause = uiNormPause(g_presets[r->itemPreset[r->os.cur]], r->pause, r->curLevel); }

// Opens the page: items = the tag's selectable presets; start = the configured preset, else the recommended one, else the first.
void uiOpenPage(UiPanel* r, const char* tag, int curLevel) {
    uiClosePage(r);
    r->curLevel = curLevel;
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
    const Preset& sp = g_presets[r->itemPreset[start]];
    r->pause = configured ? cfg->pause : uiStartPause(sp, curLevel);   // unconfigured: hold from the current level; configured: the saved pause (raised to lo below)
    uiClampPagePause(r);
    snprintf(r->pageTag, sizeof r->pageTag, "%s", tag);
    r->pageTable = g_tableStamp;
    r->open = r->nItems > 0;
    if (r->open) {
        logLine("UI page open tag=%s start=%s (%s)", tag, r->items[start].id, how);
        if (uiCanPause(sp)) logLine("UI pause lo=%d hi=%d cur=%d", uiPauseLo(sp, curLevel), uiPauseHi(sp), curLevel);
    }
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
    uiShowBack(r, true);
    if (r->failed) uiSetText(r, C_TITLE, g_ptDoc.lossy ? kLossyText : "Could not save - see log");
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
    { char bb[24]; uiSetText(r, C_BANK, uiBankText(r->bank, bb, sizeof bb)); }
    uiShow(r, C_BANK, P.holdClass != 0);
    const bool pz = uiCanPause(P) && r->bank;
    { char bb[24]; uiSetText(r, C_PAUSE, uiPauseText(P, r->pause, bb, sizeof bb)); }
    uiShow(r, C_PAUSE, pz);
    uiSetText(r, C_PMINUS, "-");
    uiShow(r, C_PMINUS, pz && uiPauseMinusShown(P, r->pause, r->curLevel));
    uiSetText(r, C_PPLUS, "+");
    uiShow(r, C_PPLUS, pz && uiPausePlusShown(P, r->pause));
    uiSetText(r, C_REC, "Use recommended");
    uiShow(r, C_REC, r->recItem >= 0 && r->os.cur != r->recItem);
    uiSetText(r, C_APPLY, "Apply");
    uiShow(r, C_APPLY, true);
    uiSetText(r, C_VANILLA, "Vanilla");
    uiShow(r, C_VANILLA, configured);
    uiSetText(r, C_CANCEL, "Cancel");
    uiShow(r, C_CANCEL, true);
}

// ---- R10 filter page (the PC, batch 3a): same controls as the options page; rows = R10Page.h ------------------------------------------
// Start state: the playthrough's `pc` line, else the ini r10* keys when r10=1, else R10Page's default.
void uiOpenR10(UiPanel* r) {
    uiClosePage(r);
    reloadPlay(true);
    const char* how = "default";
    if (const pt::Line* l = ptPcLine()) { r->r10 = r10p::fromTokens(l->r10, l->pcfilter, l->ffilter, l->pfilter); how = "playthrough"; }
    else if (g_cfg.r10) { r->r10 = r10p::fromTokens(true, g_cfg.r10Profile, r10f::modeName(g_cfg.r10Feats), r10f::modeName(g_cfg.r10Powers)); how = "ini"; }
    else r->r10 = r10p::defaultState();
    r->open = r->pcPage = true;
    r->pageTable = g_tableStamp;
    logLine("UI r10 page open start=%s on=%d", how, r->r10.on);
}
void uiRenderR10(UiPanel* r) {
    char buf[UiText];
    uiSlots(r);
    uiShowBack(r, true);
    uiSetText(r, C_TITLE, r->failed ? (g_ptDoc.lossy ? kLossyText : "Could not save - see log") : "Level-up filter");
    uiShow(r, C_TITLE, true);
    for (int row = 0; row < NumRows; ++row) {
        r10p::rowText(r->r10, row, buf, sizeof buf);
        uiSetText(r, C_ROW0 + row, buf);
        uiShow(r, C_ROW0 + row, true);
    }
    for (int i = C_BUILD; i <= C_CANCEL; ++i) if (i != C_APPLY && i != C_CANCEL) uiShow(r, i, false);
    uiSetText(r, C_APPLY, "Apply");
    uiShow(r, C_APPLY, true);
    uiSetText(r, C_CANCEL, "Cancel");
    uiShow(r, C_CANCEL, true);
}
// Row click cycles; Apply saves the `pc` line (closes on success, else the title shows the failure).
void uiClickR10(UiPanel* r, int row, int ctl) {
    if (row >= 0) { r10p::cycle(r->r10, row, +1); return; }
    if (ctl != C_APPLY) return;
    char pcf[r10p::TokenLen];
    const char* ff = nullptr; const char* pf = nullptr;
    r10p::toTokens(r->r10, pcf, sizeof pcf, &ff, &pf);
    const bool ok = ptSetPc(r->r10.on, pcf, ff, pf);
    logLine("UI apply pc r10=%d pcfilter=%s ffilter=%s pfilter=%s ok=%d", r->r10.on, pcf, ff, pf, ok);
    if (ok) uiClosePage(r); else r->failed = true;
}
// R11-style status line for the PC: the effective filter (playthrough line or ini), false = hidden (filter off).
bool buildPcStatus(char* out, size_t cap) {
    const R10Eff e = r10Effective();
    if (out && cap) out[0] = 0;
    if (!out || !cap || !e.on) return false;
    snprintf(out, cap, "Level-up filter: on - feats %s, powers %s", r10f::modeName(e.feats), r10f::modeName(e.powers));
    return true;
}
// Native tip strings for the PC or a companion (only the controls whose text differs). Called from uiTick when the shown creature's kind changes,
// never from uiClick: at that moment the cursor is on the party selector, so no control of ours owns the engine tooltip.
void uiTipKind(UiPanel* r, bool pc) {
    int nat = 0;
    for (int i = 0; i < C_N; ++i) if (r->c[i] && uiTipTextPc(i) && guitip::setNative(r->c[i], uiTipText(i, pc))) ++nat;
    r->tipPc = pc;
    logLine("UI tips kind=%s native=%d", pc ? "pc" : "companion", nat);
}
// 1 = a tagged creature (tag in out), 0 = the PC (readable creature with an empty tag: r10IsPc's rule), -1 = unreadable.
int uiReadTag(uintptr_t stats, char* out, int cap) {
    out[0] = 0;
    if (!stats) return -1;
    if (readTag(stats, out, cap)) return out[0] ? 1 : 0;
    uint32_t cr = 0, str = 0, len = 0;
    if (!readAt(S_Tag, stats, St_Creature, &cr) || !cr || !readAt(S_Tag, cr, Cr_Tag, &str) || !readAt(S_Tag, cr, Cr_Tag + 4, &len)) return -1;
    return (!str || !len) ? 0 : -1;
}
// 1/0 = BTN_LEVELUP (control index 66) visible flag; -1 = unreadable. Cross-check for the heldResult rule (logged on change only).
int uiLvlBtnVisible(const UiPanel* r) {
    uint32_t arr = 0, ctl = 0;
    int cnt = 0;
    if (!readAt(S_Ui, r->panel, guikit::Panel_CtlCount, &cnt) || cnt <= BtnLevelUpIndex || !readAt(S_Ui, r->panel, guikit::Panel_CtlArray, &arr) || !arr ||
        !readAt(S_Ui, arr, BtnLevelUpIndex * 4, &ctl) || !ctl) return -1;
    return guikit::isVisible(ctl) ? 1 : 0;
}

// ---- hover tooltips: custom fallback box (GuiTip.h; the native strings are set in uiAppend) --------------------------------------
// Box metrics in layout units (the controls' dialog font is ~6 units per char at 800x600): 36 chars + padding = 228 <= the 246-wide column.
constexpr guitip::Metrics kTipMetrics = {6, 12, 6, 5};
constexpr int kTipGap = 4;
constexpr guitip::Region kTipRegionPage = {54, 134, 300, 470};   // stats column (nothing may sit right of x 300 inside the page area)
constexpr guitip::Region kTipRegionCfg = {54, 134, 300, 372};    // page closed: the box goes left of / above the Configure button (y 372)
constexpr guitip::Region kTipRegionIcon = {350, 134, 525, 255};   // icon: inside the 3D view right of SLD_ALIGN (x 346), above BTN_AUTO (y 260)

// Rect of control i in layout units as uiPlace places it (sc: the icon's square width in units; null = its spec rect).
guitip::Rect uiRectUnits(const UiPanel* r, int i, const guikit::Scale* sc = nullptr) {
    guitip::Rect q = {kUiSpec[i].x, kUiSpec[i].y, kUiSpec[i].w, kUiSpec[i].h};
    if (i == C_ICON && sc && sc->sx > 0.f) { q.w = static_cast<int>(lroundf(kIconSide * sc->sy / sc->sx)); q.x = kIconRight - q.w; }
    if (i >= C_ROW0 && i < C_ROW0 + NumRows) q.y = kRowY0 + kRowStep * (r->rowSlot[i - C_ROW0] >= 0 ? r->rowSlot[i - C_ROW0] : 0);
    return q;
}
// Shown control with tip text under (x,y); -1 = none. Separate from uiHit: the pause label has a tip but is not clickable.
int uiTipHit(const UiPanel* r, int x, int y) {
    for (int i = 0; i < C_TIPBG; ++i) if ((r->shownMask & (1u << i)) && r->c[i] && uiTipText(i, r->tipPc) && guikit::hit(r->c[i], x, y)) return i;
    return -1;
}
void uiTipHide(UiPanel* r) { uiShow(r, C_TIPBG, false); uiShow(r, C_TIP, false); }
void uiTipReset(UiPanel* r) { guitip::reset(r->tip); r->tipProbe = guitip::ProbeSnap(); r->tipProbeKey = -1; uiTipHide(r); }
void uiPlaceUnits(UiPanel* r, int i, const guikit::Scale& sc, const guitip::Rect& q) {
    int x, y, w, h, ex, ey, ew, eh;
    guikit::unitRect(sc, q.x, q.y, q.w, q.h, &x, &y, &w, &h);
    if (!guikit::getExtent(r->c[i], &ex, &ey, &ew, &eh) || ex != x || ey != y || ew != w || eh != h) guikit::setRect(r->c[i], x, y, w, h);
}
// Every tick after the draw. hov = control with tip text under the cursor (-1 none); clicked = a press edge this tick (hides the tip).
void uiTipTick(UiPanel* r, const guikit::Scale& sc, int hov, bool clicked) {
    if (clicked || r->open != r->tipOpen) { r->tipOpen = r->open; uiTipReset(r); }
    guitip::Mgr mg;
    guitip::readMgr(&mg);
    const uintptr_t hc = hov >= 0 ? r->c[hov] : 0;
    const bool wasShown = r->tip.boxShown;
    const bool show = guitip::tick(r->tip, hov, hc && mg.ok && mg.owner == hc, GetTickCount(), guitip::delayMs(mg));
    if (hov != r->tipProbeKey) { r->tipProbeKey = hov; r->tipProbe = guitip::ProbeSnap(); }
    if (hc) {   // probe: one line per change of {over, owner, drawn, ctl bit 0x40} for the hovered control
        int fl = 0;
        guikit::detail::rdInt(hc, guikit::Ctl_Flags, &fl);
        if (guitip::probeChanged(r->tipProbe, mg, hc, fl))
            logLine("TIPPROBE over_is_ours=%d owner_is_ours=%d shown=%d ctl40=%d timer=%.2f appear=%.2f dis=%u ctl=%s", r->tipProbe.over, r->tipProbe.owner, r->tipProbe.shown, r->tipProbe.ctl40,
                    mg.timer, mg.appear, (unsigned)mg.disabled, kUiName[hov]);
    }
    if (!show || !r->c[C_TIPBG] || !r->c[C_TIP]) { if (!show) uiTipHide(r); return; }
    const char* t = uiTipText(hov, r->tipPc);
    int lines = 0, maxLen = 0;
    guitip::measure(t, &lines, &maxLen);
    const guitip::Rect b = guitip::layout(lines, maxLen, kTipMetrics, uiRectUnits(r, hov, &sc), hov == C_ICON ? kTipRegionIcon : r->open ? kTipRegionPage : kTipRegionCfg, kTipGap);
    uiPlaceUnits(r, C_TIPBG, sc, b);
    uiPlaceUnits(r, C_TIP, sc, {b.x + kTipMetrics.padX, b.y + kTipMetrics.padY, b.w - 2 * kTipMetrics.padX, b.h - 2 * kTipMetrics.padY});
    uiSetText(r, C_TIP, t);
    uiShow(r, C_TIPBG, true);
    uiShow(r, C_TIP, true);
    guikit::setClickable(r->c[C_TIPBG], false);   // the box never swallows clicks / native hover
    guikit::setClickable(r->c[C_TIP], false);
    if (!wasShown) logLine("UI tip custom ctl=%s (native box not seen)", kUiName[hov]);
}

void uiHideAll(UiPanel* r) {
    for (int i = 0; i < C_N; ++i) uiShow(r, i, false);
    if (r->open) uiClosePage(r);
    r->hover = -1;
    uiTipReset(r);
}

// ---- input ----------------------------------------------------------------------------------------------------------------
bool uiClickable(const UiPanel* r, int i) {
    if (!(r->shownMask & (1u << i))) return false;
    if (i == C_CFG) return !r->open;
    if (i == C_ICON) return true;   // batch 3d: stays on an open page and closes it
    if (!r->open) return false;
    return (i >= C_ROW0 && i < C_ROW0 + NumRows) || i == C_INSTANT || i == C_BANK || i == C_PMINUS || i == C_PPLUS || i == C_REC || i == C_APPLY || i == C_VANILLA || i == C_CANCEL;
}
// Clickable shown control under (x,y); -1 = none.
int uiHit(const UiPanel* r, int x, int y) {
    for (int i = 0; i < C_N; ++i) if (uiClickable(r, i) && r->c[i] && guikit::hit(r->c[i], x, y)) return i;
    return -1;
}

void uiClick(UiPanel* r, const char* tag, int ctl) {
    const int row = (ctl >= C_ROW0 && ctl < C_ROW0 + NumRows) ? ctl - C_ROW0 : -1;
    logLine("UI click %s tag=%s row=%s", kUiName[ctl], tag, row >= 0 ? (r->pcPage ? r10p::rowName(row) : opt::rowName(row)) : "-");
    if (ctl == C_ICON && r->open) { logLine("UI gear close tag=%s", tag); uiClosePage(r); return; }   // batch 3d: same as Cancel
    if (ctl == C_CFG || ctl == C_ICON) { if (r->pc) uiOpenR10(r); else uiOpenPage(r, tag, r->curLevel); return; }
    if (!r->open) return;
    if (ctl == C_CANCEL) { uiClosePage(r); return; }
    r->failed = false;
    if (r->pcPage) { uiClickR10(r, row, ctl); return; }
    if (row >= 0) { opt::cycle(r->os, row, +1); uiClampPagePause(r); return; }
    switch (ctl) {
    case C_INSTANT: r->instant = !r->instant; break;
    case C_BANK: uiBankToggle(g_presets[r->itemPreset[r->os.cur]], &r->bank, &r->pause, r->curLevel); break;
    case C_PMINUS: uiPauseStep(g_presets[r->itemPreset[r->os.cur]], &r->pause, -1, r->curLevel); break;
    case C_PPLUS: uiPauseStep(g_presets[r->itemPreset[r->os.cur]], &r->pause, +1, r->curLevel); break;
    case C_REC: if (r->recItem >= 0) { r->os.cur = r->recItem; uiClampPagePause(r); } break;   // re-init on the recommended item; toggles are kept (pause clamped)
    case C_APPLY: {
        const Preset& P = g_presets[r->itemPreset[r->os.cur]];
        const bool hold = P.holdClass ? r->bank : true;
        const int pause = (P.holdClass && r->bank) ? uiClampPause(P, r->pause, r->curLevel) : 0;
        const bool ok = ptApply(tag, P.id, r->instant, hold, pause);
        logLine("UI apply tag=%s preset=%s instant=%d hold=%d pause=%d ok=%d", tag, P.id, r->instant, hold, pause, ok);
        if (ok && r->instant) requestApplyNow(tag);   // consumed by the next menu-draw gate call (outside SetStats)
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
    saferead::beginScope();
    UiPanel* r = findUi(panel);
    if (!r) {
        if (!g_uiNoRecLogged) { g_uiNoRecLogged = true; logLine("UI tick: no record for panel=%08x (ctor hook not seen)", (unsigned)panel); }
        return;
    }
    { uint8_t ids[4] = {}, lv[4] = {}; int n = 0, total = 0; r->curLevel = readClasses(stats, ids, lv, &n, &total) ? total : -1; }   // refreshed before input: uiClick only touches UI state
    const bool click = guikit::clickEdge(r->edge);   // sampled every tick so a press that began elsewhere never fires
    if (!r->appended) uiAppend(r);
    embtex::heal();   // refill the embedded gear after an engine texture reset
    if (!gateActive()) { uiHideAll(r); return; }
    char tag[32];
    const int kind = uiReadTag(stats, tag, sizeof tag);
    const bool haveTag = kind >= 0, pc = kind == 0;   // batch 3a: the PC (empty tag) gets the R10 filter page
    if (!haveTag || (!pc && !uiCountSelectable(tag))) { uiHideAll(r); r->lastTag[0] = 0; return; }   // unreadable, or a tag without selectable presets
    if (strcmp(r->lastTag, tag) || r->pc != pc) { if (r->open) uiClosePage(r); snprintf(r->lastTag, sizeof r->lastTag, "%s", tag); r->pc = pc; }
    if (r->tipPc != pc) { uiTipReset(r); uiTipKind(r, pc); }
    if (r->open && !r->pcPage && (_stricmp(r->pageTag, tag) || r->pageTable != g_tableStamp)) uiClosePage(r);
    const bool lvl = heldResult != 0;   // the engine shows BTN_LEVELUP/BTN_AUTO exactly when SetStats' CanLevelUp (held) result is nonzero
    if (r->lvlPending >= 0) { logLine("UI lvlbtn shown=%d isVisible=%d", r->lvlPending, uiLvlBtnVisible(r)); r->lvlPending = -1; }   // sampled a tick late: the engine sets bit1 after this hook
    if (static_cast<int>(lvl) != r->lvlLogged) { r->lvlLogged = lvl; r->lvlPending = lvl; }
    guikit::Scale sc;
    if (!uiScale(r, &sc)) {
        if (!g_uiNoScaleLogged) { g_uiNoScaleLogged = true; logLine("UI no usable scale (BTN_LEVELUP missing, no fallback) - controls hidden"); }
        uiHideAll(r);
        return;
    }
    // input first (acts on what the previous tick drew)
    int cx = 0, cy = 0;
    const bool haveCur = guikit::cursorEngine(&cx, &cy);
    const int hit = haveCur ? uiHit(r, cx, cy) : -1;
    const bool clickable = hit >= 0 && uiClickable(r, hit);
    const bool hovered = clickable && hit == r->hover;   // the cursor was already over this control on the previous tick (guards clicks meant for a GUI on top)
    if (clickable && hit != r->hover) guikit::playHoverSound();
    r->hover = clickable ? hit : -1;
    uiIconHover(r, r->hover == C_ICON);
    uiHoverTint(r, r->hover);
    if (click && hovered) uiClick(r, tag, hit);

    uiSlots(r);
    uiPlace(r, sc);

    // draw
    if (pc) {
        if (r->open) {
            uiShow(r, C_CFG, false); uiShowIcon(r, true); uiShow(r, C_STATUS, false);
            uiRenderR10(r);
        } else {
            for (int i = C_BG0; i <= C_CANCEL; ++i) uiShow(r, i, false);
            bool hl = false;
            uiSetText(r, C_CFG, "Level-up filter");
            uiNudge(r, hl);
            uiShow(r, C_CFG, lvl);
            uiShowIcon(r, !lvl);
            char st[UiText];
            bool sv = buildPcStatus(st, sizeof st);
            if (g_ptDoc.lossy) { snprintf(st, sizeof st, "%s", kLossyText); sv = true; }
            if (sv) uiSetText(r, C_STATUS, st);
            uiShow(r, C_STATUS, sv);
        }
        uiTipTick(r, sc, haveCur ? uiTipHit(r, cx, cy) : -1, click);
        return;
    }
    const TagCfg* cfg = tagConfig(tag);
    const Preset* P = cfg && cfg->preset >= 0 && g_tableOk ? &g_presets[cfg->preset] : nullptr;
    const bool configured = P != nullptr;
    if (r->open) {
        uiShow(r, C_CFG, false);
        uiShowIcon(r, true);
        uiShow(r, C_STATUS, false);
        uiRenderPage(r, tag);
        uiTipTick(r, sc, haveCur ? uiTipHit(r, cx, cy) : -1, click);
        return;
    }
    for (int i = C_BG0; i <= C_CANCEL; ++i) uiShow(r, i, false);
    bool hl = false;
    uiSetText(r, C_CFG, cfgButtonText(configured, realCan != 0, &hl));
    uiNudge(r, hl && lvl);
    uiShow(r, C_CFG, lvl);   // batch 3a: the big button only next to the engine's level-up buttons; else the corner icon
    uiShowIcon(r, !lvl);
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
    if (g_ptDoc.lossy) { snprintf(st, sizeof st, "%s", kLossyText); sv = true; }   // F6: the file cannot be rewritten (Apply refused); say why on the screen
    if (sv) uiSetText(r, C_STATUS, st);
    uiShow(r, C_STATUS, sv);
    uiTipTick(r, sc, haveCur ? uiTipHit(r, cx, cy) : -1, click);
}
