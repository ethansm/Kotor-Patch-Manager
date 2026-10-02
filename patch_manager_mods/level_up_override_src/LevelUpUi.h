// LevelUpUi.h - M4 character-screen UI of Level-Up Override 0.3.0: Configure button (R6) with the R9 nudge, the path wizard (R8, WizardLogic.h),
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
// Layout units are the 800x600 GUI units of character_p.gui; Mod 14's panel is x 536-752, ours stays within x 54..525.

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
constexpr int NumCells = 6, MaxUiPanels = 4, BtnLevelUpIndex = 66, UiText = 160;
constexpr uint32_t UiMagic = 0x4C554931;   // 'LUI1'
enum UiCtl { C_CFG, C_STATUS, C_WBG, C_WTITLE, C_CELL0, C_WBACK = C_CELL0 + NumCells, C_WCANCEL, C_N };
struct UiSpec { bool framed; int x, y, w, h; };   // layout units (800x600)
const UiSpec kUiSpec[C_N] = {
    {true, 362, 372, 94, 26},                                                                            // cfgBtn, under BTN_LEVELUP [362,325,94,40]
    {false, 54, 476, 476, 18},                                                                           // status, between LBL_XP_BACK (bottom 470) and LBL_BAR3 (505)
    {true, 150, 140, 375, 300}, {false, 162, 148, 350, 22},                                              // wizard bg, title
    {true, 170, 178, 335, 30}, {true, 170, 214, 335, 30}, {true, 170, 250, 335, 30},                     // cells
    {true, 170, 286, 335, 30}, {true, 170, 322, 335, 30}, {true, 170, 358, 335, 30},
    {true, 170, 400, 100, 28}, {true, 405, 400, 100, 28}};                                               // Back, Cancel
const char* const kUiName[C_N] = {"cfg", "status", "wbg", "title", "cell0", "cell1", "cell2", "cell3", "cell4", "cell5", "back", "cancel"};
const char* const kTplPlain = "LBL_EXPERIENCE_STAT";
const char* const kTplFramed = "LBL_XP_BACK";
constexpr int kPlainRect[4] = {60, 449, 108, 18}, kFramedRect[4] = {54, 431, 223, 39};   // the templates' own unit rects (fallback scale)

enum UiAct { A_NONE, A_CHOOSE, A_INFO, A_INSTANT, A_HOLD, A_APPLY, A_VANILLA };

struct UiPanel {
    uint32_t magic = 0;
    uintptr_t panel = 0;
    uintptr_t c[C_N] = {};
    int nCreated = 0;
    bool appended = false, createLogged = false;
    guikit::Scale fb = {1.f, 1.f, 0.f, 0.f, 60, 449, false};   // fallback scale from a control's ctor-time extent
    char text[C_N][UiText] = {};
    uint32_t textFail = 0;
    bool colorTried = false, colorOk = false, nudgeOn = false;
    float base[3] = {};
    guikit::ClickEdge edge;
    int hover = -1;
    char lastTag[32] = {};
    // wizard
    bool wizOpen = false;
    char wizTag[32] = {};
    uint64_t wizTable = 0;
    wiz::Item items[wiz::MaxItems] = {};
    int itemPreset[wiz::MaxItems] = {};
    int nItems = 0;
    wiz::State ws = {};
    int resSeen = -1;
    bool instant = false, hold = true, failed = false;
    int act[NumCells] = {}, actArg[NumCells] = {};
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

// First tick: append every control (draw order = append order: bg before cells), set alignments, log once.
void uiAppend(UiPanel* r) {
    int ok = 0;
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i]) continue;
        if (guikit::appendToPanel(r->panel, r->c[i])) ++ok; else r->c[i] = 0;   // a failed control stays hidden and unused
    }
    r->appended = true;
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i] || i == C_WBG || i == C_STATUS) continue;
        guikit::setAlign(r->c[i], i == C_WTITLE ? 17 : 18);   // 17 top-centre, 18 centre; the status line keeps the template's left alignment
    }
    unsigned mask = 0;
    for (int i = 0; i < C_N; ++i) if (r->c[i] && guikit::detail::textObjOk(r->c[i])) mask |= 1u << i;
    logLine("UI create panel=%08x n=%d appended=%d textobj=%03x", (unsigned)r->panel, r->nCreated, ok, mask);
    r->createLogged = true;
}

// ---- placement ------------------------------------------------------------------------------------------------------------
// Scale from BTN_LEVELUP (panel control list index 66, unit rect 362,325,94,40) when its extent looks right (w/h = 2.35 +- 0.4), else the
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
void uiPlace(UiPanel* r, const guikit::Scale& s) {
    for (int i = 0; i < C_N; ++i) {
        if (!r->c[i]) continue;
        int x, y, w, h, ex, ey, ew, eh;
        guikit::unitRect(s, kUiSpec[i].x, kUiSpec[i].y, kUiSpec[i].w, kUiSpec[i].h, &x, &y, &w, &h);
        if (!guikit::getExtent(r->c[i], &ex, &ey, &ew, &eh) || ex != x || ey != y || ew != w || eh != h) guikit::setRect(r->c[i], x, y, w, h);
    }
}

// ---- small setters ------------------------------------------------------------------------------------------------------
void uiShow(UiPanel* r, int i, bool vis) { if (r->c[i]) guikit::setVisible(r->c[i], vis); }
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

// ---- wizard ---------------------------------------------------------------------------------------------------------------
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
void uiCloseWizard(UiPanel* r) {
    r->wizOpen = false;
    r->nItems = 0;
    r->ws = wiz::State();
    r->resSeen = -1;
    r->failed = false;
    for (int j = 0; j < NumCells; ++j) { r->act[j] = A_NONE; r->actArg[j] = 0; }
}
void uiOpenWizard(UiPanel* r, const char* tag) {
    uiCloseWizard(r);
    for (int i = 0; i < g_nPresets && r->nItems < wiz::MaxItems; ++i) {
        const Preset& p = g_presets[i];
        if (!p.indexed || _stricmp(p.tag, tag)) continue;
        const char* const ans[wiz::NumQuestions] = {p.role, p.style, p.alignment, p.convert, p.skills};
        if (!wiz::selectable(ans)) continue;
        wiz::Item& it = r->items[r->nItems];
        it.id = p.id;
        for (int q = 0; q < wiz::NumQuestions; ++q) it.ans[q] = ans[q];
        it.shortName = p.shortName;
        r->itemPreset[r->nItems++] = i;
    }
    wiz::reset(r->ws, r->items, r->nItems);
    snprintf(r->wizTag, sizeof r->wizTag, "%s", tag);
    r->wizTable = g_tableStamp;
    r->wizOpen = r->nItems > 0;
}
const char* uiPageName(const UiPanel* r, const wiz::View& v) {
    if (!r->wizOpen) return "-";
    return v.result ? "Result" : wiz::pageName(v.page);
}

// Draws the wizard for the current view and fills the row actions (cells 0..5).
void uiRenderWizard(UiPanel* r, const char* tag) {
    wiz::View v = wiz::view(r->ws);
    if (!v.result && v.page < 0) { uiCloseWizard(r); return; }   // no candidates (should not happen): close
    for (int j = 0; j < NumCells; ++j) { r->act[j] = A_NONE; r->actArg[j] = 0; }
    char buf[UiText], cb[16];
    if (!v.result) {
        uiSetText(r, C_WTITLE, wiz::questionText(v.page));
        for (int j = 0; j < v.nOptions && j < NumCells; ++j) {
            r->act[j] = A_CHOOSE; r->actArg[j] = j;
            uiSetText(r, C_CELL0 + j, v.page == wiz::Pick && v.optionItem[j] >= 0 ? r->items[v.optionItem[j]].shortName : wiz::answerLabel(v.page, v.options[j]));
        }
    } else {
        const wiz::Item& it = r->items[v.resultItem];
        const Preset& P = g_presets[r->itemPreset[v.resultItem]];
        const TagCfg* cfg = tagConfig(tag);
        if (r->resSeen != v.resultItem) {   // toggles start from the current config when re-configuring the same preset
            const bool same = cfg && cfg->preset == r->itemPreset[v.resultItem];
            r->instant = same ? cfg->instant : false;
            r->hold = same ? cfg->hold : true;
            r->resSeen = v.resultItem;
            r->failed = false;
        }
        uiSetText(r, C_WTITLE, r->failed ? "Could not save - see log" : it.shortName);
        int row = 0;
        snprintf(buf, sizeof buf, "Role/style: %s %s, Jedi: %s", wiz::answerLabel(wiz::Role, it.ans[wiz::Role]), wiz::answerLabel(wiz::Style, it.ans[wiz::Style]),
                 wiz::answerLabel(wiz::Convert, it.ans[wiz::Convert]));
        r->act[row] = A_INFO; uiSetText(r, C_CELL0 + row++, buf);
        r->act[row] = A_INSTANT; uiSetText(r, C_CELL0 + row++, r->instant ? "Instant level-ups: On" : "Instant level-ups: Off");
        if (P.holdClass) {
            snprintf(buf, sizeof buf, "Hold levels until %s: %s", className(P.holdClass, cb, sizeof cb), r->hold ? "On" : "Off");
            r->act[row] = A_HOLD; uiSetText(r, C_CELL0 + row++, buf);
        }
        r->act[row] = A_APPLY; uiSetText(r, C_CELL0 + row++, "Apply");
        if (cfg && cfg->preset >= 0) { r->act[row] = A_VANILLA; uiSetText(r, C_CELL0 + row++, "Use vanilla (no path)"); }
    }
    uiShow(r, C_WBG, true);
    uiShow(r, C_WTITLE, true);
    for (int j = 0; j < NumCells; ++j) uiShow(r, C_CELL0 + j, r->act[j] != A_NONE);
    uiShow(r, C_WBACK, wiz::canBack(r->ws));
    uiShow(r, C_WCANCEL, true);
    uiSetText(r, C_WBACK, "Back");
    uiSetText(r, C_WCANCEL, "Cancel");
}

void uiHideAll(UiPanel* r) {
    for (int i = 0; i < C_N; ++i) uiShow(r, i, false);
    if (r->wizOpen) uiCloseWizard(r);
    r->hover = -1;
}

// ---- input ----------------------------------------------------------------------------------------------------------------
// Top-most visible control under (x,y): wizard controls first (Back, Cancel, cells, bg), else the Configure button. -1 = none.
int uiHit(const UiPanel* r, int x, int y) {
    int order[C_N], n = 0;
    if (r->wizOpen) {
        order[n++] = C_WBACK; order[n++] = C_WCANCEL;
        for (int j = 0; j < NumCells; ++j) order[n++] = C_CELL0 + j;
        order[n++] = C_WBG;
    } else {
        order[n++] = C_CFG;
    }
    for (int k = 0; k < n; ++k) if (r->c[order[k]] && guikit::hit(r->c[order[k]], x, y)) return order[k];
    return -1;
}
bool uiClickable(const UiPanel* r, int i) {
    if (i == C_CFG || i == C_WBACK || i == C_WCANCEL) return true;
    if (i >= C_CELL0 && i < C_CELL0 + NumCells) { const int a = r->act[i - C_CELL0]; return a != A_NONE && a != A_INFO; }
    return false;
}

void uiClick(UiPanel* r, const char* tag, int ctl) {
    const wiz::View v = wiz::view(r->ws);
    logLine("UI click %s tag=%s page=%s", kUiName[ctl], tag, uiPageName(r, v));
    if (ctl == C_CFG) { uiOpenWizard(r, tag); return; }
    if (!r->wizOpen) return;
    if (ctl == C_WCANCEL) { uiCloseWizard(r); return; }
    if (ctl == C_WBACK) { wiz::back(r->ws); r->failed = false; return; }
    if (ctl < C_CELL0 || ctl >= C_CELL0 + NumCells) return;
    const int j = ctl - C_CELL0;
    switch (r->act[j]) {
    case A_CHOOSE: wiz::choose(r->ws, r->actArg[j]); break;
    case A_INSTANT: r->instant = !r->instant; break;
    case A_HOLD: r->hold = !r->hold; break;
    case A_APPLY:
        if (v.result && v.resultItem >= 0 && v.resultItem < r->nItems) {
            const Preset& P = g_presets[r->itemPreset[v.resultItem]];
            const bool hold = P.holdClass ? r->hold : true;
            const bool ok = ptApply(tag, P.id, r->instant, hold);
            logLine("UI apply tag=%s preset=%s instant=%d hold=%d ok=%d", tag, P.id, r->instant, hold, ok);
            if (ok) uiCloseWizard(r); else r->failed = true;
        }
        break;
    case A_VANILLA: {
        const bool ok = ptRemove(tag);
        logLine("UI vanilla tag=%s ok=%d", tag, ok);
        if (ok) uiCloseWizard(r); else r->failed = true;
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
    if (strcmp(r->lastTag, tag)) { if (r->wizOpen) uiCloseWizard(r); snprintf(r->lastTag, sizeof r->lastTag, "%s", tag); }
    if (r->wizOpen && (_stricmp(r->wizTag, tag) || r->wizTable != g_tableStamp)) uiCloseWizard(r);
    guikit::Scale sc;
    if (!uiScale(r, &sc)) {
        if (!g_uiNoScaleLogged) { g_uiNoScaleLogged = true; logLine("UI no usable scale (BTN_LEVELUP missing, no fallback) - controls hidden"); }
        uiHideAll(r);
        return;
    }
    uiPlace(r, sc);

    // input first (acts on what the previous tick drew)
    int cx = 0, cy = 0;
    const int hit = guikit::cursorEngine(&cx, &cy) ? uiHit(r, cx, cy) : -1;
    const bool clickable = hit >= 0 && uiClickable(r, hit);
    if (clickable && hit != r->hover) guikit::playHoverSound();
    r->hover = clickable ? hit : -1;
    if (click && clickable) uiClick(r, tag, hit);

    // draw
    const TagCfg* cfg = tagConfig(tag);
    const Preset* P = cfg && cfg->preset >= 0 && g_tableOk ? &g_presets[cfg->preset] : nullptr;
    const bool configured = P != nullptr;
    if (r->wizOpen) {
        uiShow(r, C_CFG, false);
        uiShow(r, C_STATUS, false);
        uiRenderWizard(r, tag);
        return;
    }
    for (int i = C_WBG; i < C_N; ++i) uiShow(r, i, false);
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
    const bool sv = configured && buildStatus(true, true, held, banked, P->holdClass, P->shortName, cfg->instant, st, sizeof st);
    if (sv) uiSetText(r, C_STATUS, st);
    uiShow(r, C_STATUS, sv);
}
