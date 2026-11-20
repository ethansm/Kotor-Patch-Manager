// RichMenuTypography.cpp -- KPM DLL entry and per-glyph inline color, description listbox & static menu hierarchy hooks (Doc 53 RMT1..RMT3, theme adaptation + textScalar resolution scaling + dynamic gap budgeting + rule continuation fix + full listbox item width + physical-px row heights).
#include "RichMenuTypography.h"
#include "../Common/RichDescBox.h"
#include "../Common/RichTextClassifier.h"
#include <windows.h>
#include <cstdint>
#include <cstring>
#include "../Common/KpLog.h"

KP_LOG_PATCH("rich-menu-typography");
KP_CHANNEL(chMain, "main");

#ifndef _WIN32
#ifndef GetPrivateProfileIntA
inline unsigned int GetPrivateProfileIntA(const char* /*section*/, const char* /*key*/, int def, const char* /*path*/) {
    return static_cast<unsigned int>(def);
}
#endif
#ifndef GetPrivateProfileStringA
inline unsigned long GetPrivateProfileStringA(const char* /*section*/, const char* /*key*/, const char* def, char* out, unsigned long size, const char* /*path*/) {
    if (!out || size == 0) return 0;
    if (!def) { out[0] = 0; return 0; }
    strncpy(out, def, size - 1);
    out[size - 1] = 0;
    return static_cast<unsigned long>(strlen(out));
}
#endif
#endif

#define INIW_FILE_API 1
#define MCDECL_FILE_API 1
#include "../Common/ModConfigApi.h"
#include "../Common/ProviderClient.h"
#include "../Common/ModConfigDecl.h"
#include "RichMenuTypographyPage.h"
#include "RichMenuTypographyPreview.h"

bool itemTitleFn(void* listbox, char* out, int cap);   // defined with the resolver below; the page toggle installs it

namespace {
bool g_inlineColoring = true;
bool g_richDescBox = true;
bool g_itemNameTitle = true;   // [General] ItemNameTitle (default 1): the description title row shows the selected item's name
bool g_statHierarchy = true;
bool g_verbose = false;
bool g_statsLogged = false;
uintptr_t g_lastStatsPanel = 0;
uint32_t g_lastStatsStyled = 0;
uint32_t g_overrideCount = 0;
uint32_t g_descPopulateCount = 0;
uint32_t g_statStyleCount = 0;
uint32_t g_initControlLogs = 0;

mcdecl::Page g_page;
k2mc::ConfigClient g_mcCfg;
k2mc::MenuClient g_mcMenu;
k2pc::RegisterOnce g_mcReg;
K2ModConfigEntry g_mcEntry;
bool g_pageLoaded = false;
rmtpage::Live g_live;
unsigned g_pageLogs = 0;

void mcLog(int level, const char* msg) {
    if (level == KPLOG_INFO) {
        if (KpLog_HasCore()) KP_LOG(chMain, "PAGE registered");
    } else {
        if (KpLog_HasCore()) KP_LOGV(chMain, level, "%s", msg);
    }
}

void probeLogForwarder(const char* msg) {
    if (KpLog_HasCore() && msg) {   // gated by ini ProbeRmt (g_probeLog is null when off), independent of Verbose
        KP_LOGV(chMain, KPLOG_DEBUG, "%s", msg);
    }
}

// ---- the page's own preview area (top of the right half; the bottom stays mod-config's description pane for the hovered row) ----
// Four labels (title, subtitle, rules, body) drawn with the same fonts and colours the description box uses, stacked by rmtprev::stackRows.
// Without SetDescSplit (older mod-config) there is no preview area and showPreview falls back to writing the sample into the description pane.
struct PagePreview {
    void* panel = nullptr;
    uint32_t page = 0;
    bool on = false;
    uintptr_t lbl[rmtprev::kRows] = {};
    uintptr_t div[rmtprev::kDivs] = {};   // optional divider labels (best effort: the engine may cap how many custom items a pane lists)
    int nDiv = 0;
    const char* text = nullptr;   // the sample being shown (static string from previewFor)
    uint32_t sig = 0;             // hash of text + pane + fonts: layout reruns only when it changes
    unsigned logs = 0;
};
PagePreview g_pv;

uint32_t pvHash(uint32_t h, const void* p, size_t n) {
    const unsigned char* b = static_cast<const unsigned char*>(p);
    for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 16777619u; }
    return h;
}

// Runs inside onBuilt (Options ctor window: labels can still be created from the layout template).
void pvBuild(uint32_t page, void* panel) {
    g_pv = PagePreview();
    if (!panel || !g_mcMenu.ok() || !g_mcMenu.p) return;
    if (!g_mcMenu.hasDescSplit()) {
        if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_INFO, "PAGE preview: provider has no SetDescSplit; sample goes to the description pane");
        return;
    }
    const uintptr_t pn = reinterpret_cast<uintptr_t>(panel);
    uintptr_t made[rmtprev::kRows] = {};
    int n = 0;
    for (; n < rmtprev::kRows; ++n) {
        int why = guikit::CF_None;
        uint32_t bv = 0;
        made[n] = guikit::createLabelFromTemplate(pn, "LBL_TITLE", &why, &bv);
        if (!made[n]) {
            if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_WARN, "PAGE preview: label %d create failed (why=%d vtable=%08x)", n, why, (unsigned)bv);
            break;
        }
        (void)guikit::setVisible(made[n], false);
        (void)guikit::setClickable(made[n], false);
        (void)guikit::setFill(made[n], nullptr);
        (void)guikit::detail::wr<uint32_t>(made[n] + guikit::Label_Border, guikit::Border_FillStyle, 0);
        (void)guikit::setText(made[n], "");   // the clone carries the Options title text
    }
    int listed = 0;
    if (n == rmtprev::kRows) {
        for (; listed < rmtprev::kRows; ++listed)
            if (g_mcMenu.p->AddCustom(page, reinterpret_cast<void*>(made[listed]), MUI_NOFREE) < 0) break;
    }
    for (int j = listed; j < n; ++j) guikit::releaseUnlinkedLabel(made[j], guikit::LabelVtable);   // created, never listed
    if (n != rmtprev::kRows || listed != rmtprev::kRows) {
        if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_WARN, "PAGE preview skipped: created %d listed %d of %d", n, listed, rmtprev::kRows);
        return;
    }
    // Divider labels: best effort. A failure only means the preview shows no dividers.
    uintptr_t divMade[rmtprev::kDivs] = {};
    int nDivMade = 0;
    for (; nDivMade < rmtprev::kDivs; ++nDivMade) {
        int why = guikit::CF_None;
        uint32_t bv = 0;
        const uintptr_t l = guikit::createLabelFromTemplate(pn, "LBL_TITLE", &why, &bv);
        if (!l) break;
        (void)guikit::setVisible(l, false);
        (void)guikit::setClickable(l, false);
        (void)guikit::setText(l, "");
        if (g_mcMenu.p->AddCustom(page, reinterpret_cast<void*>(l), MUI_NOFREE) < 0) {
            guikit::releaseUnlinkedLabel(l, guikit::LabelVtable);
            break;
        }
        divMade[nDivMade] = l;
    }
    if (nDivMade < rmtprev::kDivs && KpLog_HasCore())
        KP_LOGV(chMain, KPLOG_WARN, "PAGE preview: only %d of %d divider labels listed; the preview draws fewer dividers", nDivMade, rmtprev::kDivs);
    if (!g_mcMenu.p->SetDescSplit(page, rmtprev::kSplitPermille)) {
        if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_WARN, "PAGE preview skipped: SetDescSplit(%u, %d) refused", page, rmtprev::kSplitPermille);
        return;
    }
    g_pv.panel = panel;
    g_pv.page = page;
    for (int i = 0; i < rmtprev::kRows; ++i) g_pv.lbl[i] = made[i];
    for (int i = 0; i < nDivMade; ++i) g_pv.div[i] = divMade[i];
    g_pv.nDiv = nDivMade;
    g_pv.on = true;
    if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_INFO, "PAGE preview BUILD ok: %d labels + %d dividers, split %d permille", rmtprev::kRows, nDivMade, rmtprev::kSplitPermille);
}

void pvRefresh() {
    if (!g_pv.on || !g_mcMenu.p) return;
    K2MuiRect pr = {};
    if (!g_mcMenu.p->PaneRect(g_pv.panel, MUI_PANE_PREVIEW, &pr) || pr.w <= 0 || pr.h <= 0) return;
    const char* text = g_pv.text ? g_pv.text : rmtpage::kPreviewShowcase;
    const char* fonts[rmtprev::kRows] = { rmt::kFontTitle, rmt::kFontSub, rmt::getRulesFont(), rmt::getBodyFont() };
    uint32_t sig = pvHash(2166136261u, text, strlen(text));
    sig = pvHash(sig, &pr, sizeof(pr));
    for (int i = 2; i < rmtprev::kRows; ++i) sig = pvHash(sig, fonts[i], strlen(fonts[i]));
    const int dmode = rmt::dividerMode();
    sig = pvHash(sig, &dmode, sizeof(dmode));
    const int sem = rmt::semanticColors() ? 1 : 0;
    sig = pvHash(sig, &sem, sizeof(sem));
    const int rich = g_richDescBox ? 1 : 0;
    sig = pvHash(sig, &rich, sizeof(rich));
    const int itl = g_live.itemTitle ? 1 : 0;
    sig = pvHash(sig, &itl, sizeof(itl));
    if (sig == g_pv.sig) return;

    static rmt::DescSections sec;
    sec = rmt::DescSections();
    // Rich description box OFF: the game draws the vanilla single block (default font, body colour), so the preview does the same.
    const bool split = g_richDescBox && rmt::splitDescSections(text, &sec);
    if (g_richDescBox && !split) return;   // the in-game box would fall back to vanilla here; keep the previous labels and retry on the next change
    g_pv.sig = sig;
    const bool hasSub = split && sec.subtitle[0] && strcmp(sec.subtitle, sec.title) != 0;
    const char* texts[rmtprev::kRows] = { split ? rmtpage::previewTitleFor(g_live.itemTitle, g_live.sampleIdx, sec.title) : "", hasSub ? sec.subtitle : "", split ? sec.rules : "", split ? sec.body : text };
    if (!split) fonts[3] = rmt::kFontDesc;
    const float* cols[rmtprev::kRows] = { rmt::kCream, rmt::kTealDim, rmt::kTealText, rmt::kTealText };

    const int w = pr.w - 2 * rmtprev::kPad;
    int h[rmtprev::kRows] = {};
    for (int i = 0; i < rmtprev::kRows; ++i) {
        if (!texts[i][0] || w <= 0) continue;
        (void)guikit::setFont(g_pv.lbl[i], fonts[i]);
        (void)guikit::setAlign(g_pv.lbl[i], rmt::kAlignTopLeft);
        (void)guikit::setText(g_pv.lbl[i], texts[i]);
        h[i] = rmt::measureTextHeight(g_pv.lbl[i], texts[i], w, fonts[i]);
    }
    const rmtprev::Stack st = rmtprev::stackRows(rmtprev::Rect{ pr.x, pr.y, pr.w, pr.h }, h);
    for (int i = 0; i < rmtprev::kRows; ++i) {
        const uintptr_t l = g_pv.lbl[i];
        if (!st.show[i]) {
            (void)guikit::setText(l, "");
            (void)guikit::setVisible(l, false);
            continue;
        }
        (void)guikit::setRect(l, st.r[i].x, st.r[i].y, st.r[i].w, st.r[i].h);
        (void)guikit::setTextColor(l, cols[i][0], cols[i][1], cols[i][2]);
        (void)guikit::setVisible(l, true);
        (void)guikit::setClickable(l, false);
    }
    // Dividers sit in the gaps between shown rows (same modes as the in-game description box).
    const rmtprev::Divs dv = rmtprev::placeDividers(st, dmode);
    void* const divImg = (dv.count > 0) ? guikit::imageFor(rmt::kDividerFillResRef) : nullptr;
    for (int k = 0; k < g_pv.nDiv; ++k) {
        const uintptr_t l = g_pv.div[k];
        if (!l) continue;
        if (!dv.on[k] || !divImg) { (void)guikit::setVisible(l, false); continue; }
        (void)guikit::setText(l, "");
        (void)guikit::setFill(l, divImg, rmt::kDividerTint);
        (void)guikit::setFillStyle(l, 2);
        (void)guikit::setBorderTint(l, rmt::kDividerTint);
        (void)guikit::setAlpha(l, 1.0f);
        (void)guikit::setRect(l, pr.x + rmtprev::kPad, dv.y[k], pr.w - 2 * rmtprev::kPad, rmtprev::kDivH);
        (void)guikit::setVisible(l, true);
        (void)guikit::setClickable(l, false);
    }
    if (g_pv.logs < 16) {
        ++g_pv.logs;
        if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_INFO, "PAGE preview pane=%d,%d %dx%d h=%d/%d/%d/%d shown=%d dropped=%d dividers=%d/%d mode=%d sem=%d",
                                      pr.x, pr.y, pr.w, pr.h, h[0], h[1], h[2], h[3], st.shown, st.dropped, dv.count, g_pv.nDiv, dmode, sem);
    }
}

// Show the sample item description matching the row that changed. With a preview area it is drawn there and the description pane keeps the
// hovered row's help; without one it falls back to the description pane (through RichDescBox).
void showPreview(const char* rowId) {
    (void)rowId;   // one sample for every row (the Preview sample row picks it) so all options act on the same text
    g_pv.text = rmtpage::previewFor(g_live.sampleIdx);
    if (g_pv.on) { pvRefresh(); return; }
    if (g_mcMenu.ok() && g_mcMenu.p && g_mcMenu.p->SetDesc) {
        g_mcMenu.p->SetDesc(g_page.page, g_pv.text);
    }
}

// Make the font rows, the live mirror and the engine fonts agree. An unknown name in the
// ini (bad spelling, old build) maps to index 0 AND the engine font is reset to match.
void syncFontRows(const char* why) {
    const char* bodyIn = rmt::getBodyFont();
    const char* rulesIn = rmt::getRulesFont();
    g_live.fontIdx = rmtpage::fontIndexFor(bodyIn);
    g_live.rulesFontIdx = rmtpage::fontIndexFor(rulesIn);
    rmt::setBodyFont(rmtpage::fontName(g_live.fontIdx));
    rmt::setRulesFont(rmtpage::fontName(g_live.rulesFontIdx));
    g_page.setValue(rmtpage::kRowSample, g_live.sampleIdx);
    g_page.setValue(rmtpage::kRowFont, g_live.fontIdx);
    g_page.setValue(rmtpage::kRowRulesFont, g_live.rulesFontIdx);
    g_live.dividers = rmt::dividerMode();
    g_page.setValue(rmtpage::kRowDividers, g_live.dividers);
    if (KpLog_HasCore() && g_verbose) {
        KP_LOGV(chMain, KPLOG_DEBUG, "PAGE sync(%s) body=%s->%d rules=%s->%d", why,
                bodyIn ? bodyIn : "", g_live.fontIdx, rulesIn ? rulesIn : "", g_live.rulesFontIdx);
    }
}

int32_t pageOnChange(void* /*user*/, int /*row*/, const char* id, int32_t requested, int /*dir*/) {
    if (id && strcmp(id, rmtpage::kRowReset) == 0 && requested != 0) {
        g_live.inlineColoring = true;
        g_live.richDesc = true;
        g_live.stats = true;
        g_live.semantic = true;
        rmt::semanticColors() = true;
        g_live.dividers = 1;
        rmt::dividerMode() = 1;
        g_live.fontIdx = 0;
        g_live.rulesFontIdx = 0;
        g_live.itemTitle = true;
        g_itemNameTitle = true;
        rmt::g_itemTitleFn = itemTitleFn;
        g_inlineColoring = true;
        g_richDescBox = true;
        g_statHierarchy = true;
        rmt::setBodyFont(rmtpage::fontName(0));
        rmt::setRulesFont(rmtpage::fontName(0));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "InlineColoring", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "RichDescBox", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "StatHierarchy", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "SemanticColors", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "DividerMode", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "ItemNameTitle", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "BodyFont", rmtpage::fontName(0));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "RulesFont", rmtpage::fontName(0));
        g_page.setValue(rmtpage::kRowInline, 1);
        g_page.setValue(rmtpage::kRowRichDesc, 1);
        g_page.setValue(rmtpage::kRowStats, 1);
        g_page.setValue(rmtpage::kRowSemantic, 1);
        g_page.setValue(rmtpage::kRowDividers, 1);
        g_page.setValue(rmtpage::kRowItemTitle, 1);
        g_page.setValue(rmtpage::kRowFont, 0);
        g_page.setValue(rmtpage::kRowRulesFont, 0);
        g_page.setValue(rmtpage::kRowReset, 0);
        if (KpLog_HasCore()) KP_LOG(chMain, "PAGE reset to factory defaults");
        showPreview(nullptr);
        return 0;
    }
    const int32_t fin = rmtpage::applyRow(g_live, id, requested);
    g_inlineColoring = g_live.inlineColoring;
    g_richDescBox = g_live.richDesc;
    g_statHierarchy = g_live.stats;
    rmt::semanticColors() = g_live.semantic;
    rmt::dividerMode() = g_live.dividers;
    g_itemNameTitle = g_live.itemTitle;
    rmt::g_itemTitleFn = g_itemNameTitle ? itemTitleFn : nullptr;
    if (id && strcmp(id, rmtpage::kRowItemTitle) == 0) {
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "ItemNameTitle", g_live.itemTitle ? "1" : "0");
    }
    if (id && strcmp(id, rmtpage::kRowDividers) == 0) {
        char dv[4] = { static_cast<char>('0' + g_live.dividers), 0, 0, 0 };
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "DividerMode", dv);
    }
    if (id && strcmp(id, rmtpage::kRowFont) == 0) {
        rmt::setBodyFont(rmtpage::fontName(g_live.fontIdx));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "BodyFont", rmtpage::fontName(g_live.fontIdx));
    }
    if (id && strcmp(id, rmtpage::kRowRulesFont) == 0) {
        rmt::setRulesFont(rmtpage::fontName(g_live.rulesFontIdx));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "RulesFont", rmtpage::fontName(g_live.rulesFontIdx));
    }
    showPreview(id);
    if (g_pageLogs < 20) {
        ++g_pageLogs;
        if (KpLog_HasCore()) {
            KP_LOG(chMain, "PAGE row=%s req=%d final=%d", id ? id : "", static_cast<int>(requested), static_cast<int>(fin));
        }
    }
    return fin;
}

void pageOnEnter(void* /*user*/) {
    syncFontRows("enter");
    g_pv.sig = 0;   // mod-config shows every page item on entry: lay the preview out again
    showPreview(nullptr);
}

void pageOnTick(void* /*user*/) {
    pvRefresh();
}

void pageOnBuilt(void* /*user*/, uint32_t page, void* panel) {
    syncFontRows("built");
    pvBuild(page, panel);
}

void pageInit(const KPatchApi* api) {
    g_mcCfg.init(api);
    g_mcMenu.init(api);
    g_mcCfg.onConnect = [](bool ok, const char* why) {
        if (KpLog_HasCore()) KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "PAGE k2.modconfig %s: %s", ok ? "ok" : "unavailable", why);
    };
    g_mcMenu.onConnect = [](bool ok, const char* why) {
        if (KpLog_HasCore()) KP_LOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "PAGE k2.menuui %s: %s", ok ? "ok" : "unavailable", why);
    };
    char err[96] = {};
    g_pageLoaded = g_page.load(rmtpage::kPageSchema, strlen(rmtpage::kPageSchema), err, sizeof(err));
    if (!g_pageLoaded) {
        if (KpLog_HasCore()) KP_LOGV(chMain, KPLOG_WARN, "PAGE schema rejected: %s", err);
        return;
    }
    g_page.menu = &g_mcMenu;
    g_page.io = mcdecl::fileIo();
    g_page.hooks.onChange = pageOnChange;
    g_page.hooks.onEnter = pageOnEnter;
    g_page.hooks.onBuilt = pageOnBuilt;
    g_page.hooks.onTick = pageOnTick;
    g_mcEntry = g_page.entry(rmtpage::kEntryId);

    g_live.inlineColoring = g_inlineColoring;
    g_live.richDesc = g_richDescBox;
    g_live.stats = g_statHierarchy;
    g_live.semantic = rmt::semanticColors();
    g_live.dividers = rmt::dividerMode();
    g_live.fontIdx = rmtpage::fontIndexFor(rmt::getBodyFont());
    g_live.rulesFontIdx = rmtpage::fontIndexFor(rmt::getRulesFont());
    g_live.itemTitle = g_itemNameTitle;
}

void pagePoll() {
    if (!g_pageLoaded || g_mcReg.registered) return;
    g_mcCfg.poll();
    g_mcMenu.poll();
    static unsigned s_polls = 0;
    if (++s_polls == 600 && !(g_mcCfg.ok() && g_mcMenu.ok() && g_mcMenu.hasRows())) {
        if (KpLog_HasCore()) {
            KP_LOGV(chMain, KPLOG_WARN, "PAGE not registered after %u polls: modconfig=%d menuui=%d hasRows=%d",
                    s_polls, (int)g_mcCfg.ok(), (int)g_mcMenu.ok(), (int)g_mcMenu.hasRows());
        }
    }
    if (!g_mcCfg.ok() || !g_mcMenu.ok() || !g_mcMenu.hasRows()) return;
    k2pc::registerOnce(g_mcReg, g_mcCfg, g_mcEntry, mcLog);
}

constexpr size_t kCharCreamOffsets[] = {
    rmt::kCharOff_LblTitle,
    rmt::kCharOff_LblDefenseStat,
    rmt::kCharOff_LblExperienceStat,
    rmt::kCharOff_LblNeededXp,
    rmt::kCharOff_LblForceMastery,
};

constexpr size_t kCharTealDimOffsets[] = {
    rmt::kCharOff_LblStrength,
    rmt::kCharOff_LblDexterity,
    rmt::kCharOff_LblConstitution,
    rmt::kCharOff_LblIntelligence,
    rmt::kCharOff_LblWisdom,
    rmt::kCharOff_LblCharisma,
    rmt::kCharOff_LblVitality,
    rmt::kCharOff_LblForce,
    rmt::kCharOff_LblDefense,
    rmt::kCharOff_LblFortitude,
    rmt::kCharOff_LblReflex,
    rmt::kCharOff_LblWill,
    rmt::kCharOff_LblExperience,
    rmt::kCharOff_LblNextLevel,
    rmt::kCharOff_LblMore,
    rmt::kCharOff_LblLight,
    rmt::kCharOff_LblDark,
};

constexpr const char* kTitleOrValueTags[] = {
    // Primary menu panel headers / titles
    "LBL_TITLE",
    "LBL_INV",
    "LBL_ABILITIES",
    "LBL_MESSAGES",
    "LBL_Map",
    "LBL_Area",
    "LBL_BUYSELL",
    "LBL_MESSAGE",
    "LBL_NPC_NAME",
    // Inventory / Store / Container / Abilities / Party primary values
    "LBL_CREDITS_VALUE",
    "LBL_COST_VALUE",
    "LBL_STOCK_VALUE",
    "LBL_ONHAND_VALUE",
    "LBL_RANKVAL",
    "LBL_BONUSVAL",
    "LBL_TOTALVAL",
    "LBL_COUNT",
    // Character sheet primary stat values
    "LBL_DEFENSE_STAT",
    "LBL_VITALITY_STAT",
    "LBL_FORCE_STAT",
    "LBL_FORTITUDE_STAT",
    "LBL_REFLEX_STAT",
    "LBL_WILL_STAT",
    "LBL_EXPERIENCE_STAT",
    "LBL_NEEDED_XP",
    "LBL_FORCEMASTERY",
    "LBL_STR",
    "LBL_DEX",
    "LBL_CON",
    "LBL_INT",
    "LBL_WIS",
    "LBL_CHA",
    "LBL_STR_MOD",
    "LBL_DEX_MOD",
    "LBL_CON_MOD",
    "LBL_INT_MOD",
    "LBL_WIS_MOD",
    "LBL_CHA_MOD",
    // Equipment combat summary values
    "LBL_DAMTEXT",
    "LBL_DEF",
    "LBL_ATKL",
    "LBL_ATKR",
    "LBL_TOHITL",
    "LBL_TOHITR",
    "LBL_ATKL2",
    "LBL_ATKR2",
    "LBL_TOHITL2",
    "LBL_TOHITR2",
};

constexpr const char* kCaptionKeyTags[] = {
    // Character sheet static attribute / save / XP / alignment captions
    "LBL_STRENGTH",
    "LBL_DEXTERITY",
    "LBL_CONSTITUTION",
    "LBL_INTELLIGENCE",
    "LBL_WISDOM",
    "LBL_CHARISMA",
    "LBL_VITALITY",
    "LBL_FORCE",
    "LBL_DEFENSE",
    "LBL_FORTITUDE",
    "LBL_REFLEX",
    "LBL_WILL",
    "LBL_EXPERIENCE",
    "LBL_NEXT_LEVEL",
    "LBL_MORE",
    "LBL_LIGHT",
    "LBL_DARK",
    // Inventory / Store / Container static captions
    "LBL_CREDITS",
    "LBL_COST",
    "LBL_STOCK",
    "LBL_ONHAND",
    "LBL_FILTER",
    // Equipment static section & column headers
    "LBL_ATTACK_INFO",
    "LBL_DEF_INFO",
    "LBL_DEF_TEXT",
    "LBL_TOHIT",
    "LBL_DAMAGE",
    "LBL_SLOTNAME",
    "LBL_ATTACKMOD",
    // Abilities / Party / Journal static captions
    "LBL_SKILLRANK",
    "LBL_BONUS",
    "LBL_TOTAL",
    "LBL_NAME",
    "LBL_AVAILABLE",
    "LBL_NPC_LEVEL",
    "LBL_ITEM_DESCRIPTION",
};

// keepVanillaNonTeal: leave a label alone when its vanilla colour is a non-teal, non-cream colour (store gold, crafting cyan),
// so those menus keep the vanilla feel. Teal and cream labels are restyled as before. Semantic +N/-N colours never pass this.
inline bool styleLabelColor(uintptr_t ctl, const float rgb[3], bool keepVanillaNonTeal = false) noexcept {
    if (!ctl || !rgb) return false;
    uint32_t vt = 0;
    const uint32_t wantLabelVt = guikit::detail::vtab(&K2AddrTable::LabelVtable);
    if (!wantLabelVt || !guikit::detail::rd32(ctl, 0, &vt) || vt != wantLabelVt) {
        return false;
    }
    if (keepVanillaNonTeal) {
        float cur[3] = {0.0f, 0.0f, 0.0f};
        if (guikit::getTextColor(ctl, cur)) {
            const bool cream = (cur[0] > 0.78f && cur[0] < 0.82f && cur[1] > 0.78f && cur[1] < 0.82f && cur[2] > 0.68f && cur[2] < 0.72f);
            if (!rmt::isDefaultTeal(cur[0], cur[1], cur[2]) && !cream) return false;
        }
    }
    return guikit::setTextColor(ctl, rgb[0], rgb[1], rgb[2]);
}

} // namespace

namespace rmt {

MenuLabelRole classifyMenuLabelTag(const char* tag) noexcept {
    if (!tag || !*tag) return MenuLabelRole::None;
    for (const char* t : kTitleOrValueTags) {
        if (strcmp(tag, t) == 0) return MenuLabelRole::TitleOrValue;
    }
    for (const char* t : kCaptionKeyTags) {
        if (strcmp(tag, t) == 0) return MenuLabelRole::CaptionKey;
    }
    return MenuLabelRole::None;
}

void setInlineColoring(bool enabled) noexcept { g_inlineColoring = enabled; }
bool isInlineColoring() noexcept { return g_inlineColoring; }
void setRichDescBox(bool enabled) noexcept { g_richDescBox = enabled; }
bool isRichDescBox() noexcept { return g_richDescBox; }
void setStatHierarchy(bool enabled) noexcept { g_statHierarchy = enabled; }
bool isStatHierarchy() noexcept { return g_statHierarchy; }
void setVerbose(bool verbose) noexcept {
    g_verbose = verbose;  // RMTPROBE logging is gated by ini ProbeRmt, not Verbose
}
bool isVerbose() noexcept { return g_verbose; }
void resetOverrideCount() noexcept { g_overrideCount = 0; }
uint32_t getOverrideCount() noexcept { return g_overrideCount; }
void resetDescPopulateCount() noexcept { g_descPopulateCount = 0; }
uint32_t getDescPopulateCount() noexcept { return g_descPopulateCount; }
void resetStatStyleCount() noexcept { g_statStyleCount = 0; }
uint32_t getStatStyleCount() noexcept { return g_statStyleCount; }
void resetLogStateForTest() noexcept {
    g_overrideCount = 0;
    g_descPopulateCount = 0;
    g_statStyleCount = 0;
    g_initControlLogs = 0;
    g_pageLogs = 0;
    g_statsLogged = false;
    g_lastStatsPanel = 0;
    g_lastStatsStyled = 0;
}
bool pageRegistered() noexcept { return g_mcReg.registered; }
void pagePollForTest() noexcept { pagePoll(); }
int32_t applyPageRowForTest(const char* id, int32_t requested) noexcept {
    return pageOnChange(nullptr, 0, id, requested, 0);
}
void pageOnEnterForTest() noexcept { pageOnEnter(nullptr); }
int32_t getPageRowValueForTest(const char* id) noexcept { return g_page.get(id); }
} // namespace rmt

extern "C" void __cdecl RmtOverrideGlyphColor(uintptr_t ebp) {
    if (!g_inlineColoring || !ebp) {
        return;
    }

    const uint8_t* const self = *reinterpret_cast<const uint8_t* const*>(ebp - 0x168);
    const char* const curChar = *reinterpret_cast<const char* const*>(ebp - 0x10);
    if (!self || !curChar) {
        return;
    }

    const int lineIdx = *reinterpret_cast<const int*>(ebp - 0x04);
    const uint32_t baseCol = *reinterpret_cast<const uint32_t*>(ebp - 0x68);
    uint32_t* const pActiveCol = reinterpret_cast<uint32_t*>(ebp - 0xF4);

    const uint32_t newCol = rmt::resolveGlyphColor(self, curChar, lineIdx, baseCol);
    *pActiveCol = newCol;

    if (newCol != baseCol && g_verbose) {
        if (g_overrideCount < 50) {
            ++g_overrideCount;
            if (KpLog_HasCore()) {
                KP_LOGV(chMain, KPLOG_DEBUG, "override '%c' line=%d 0x%08X -> 0x%08X",
                        *curChar, lineIdx, baseCol, newCol);
            }
        }
    }
}

// P2a/P2a2 probe (doc 53 phase 2, log only; P2a2 tests the New-id-list hypothesis): which panel owns the listbox being populated, what its selected-item id is, and the engine's name for
// that item. Recipe from holster-editor ITEMNAME: client item = ClientGetItemById([[AppGlobal]+4], id), name = CSWCItem::GetName(identified=0)
// (returns "{NN}Name"). The inventory panel keeps the selected id at +0x24C8 (ISF zeroes it to 0x7F000000 during its populate), and the
// hover description is set by OnControlEntered, so the 8 words around it are logged too to find a hovered-row id. Logs only on a change,
// at most kProbeMaxLines lines. Ini [General] ProbeItemName (default 0; set 1 to turn the log-only probe on).
bool g_probeItemName = false;
constexpr uint32_t kInventoryVtable = static_cast<uint32_t>(gameaddr::InventoryPanelVtable);
constexpr int kPanelSelIdOff = 0x24C8, kPanelWinOff = 0x24C0, kProbeWinWords = 8, kProbeMaxLines = 160, kProbeOtherMax = 40, kPanelNewListOff = 0x24BC;   // W3-B: (ptr,count,capacity) list of 'New' item ids that OnControlEntered appends to

// ItemNameTitle resolver (doc 53 phase 2): the selected inventory item's display name, or false. No logging, no throttle state, independent of the
// log-only ProbeItemName flag. Same recipe as probeItemName (panel +0x24C8 selected id -> ClientGetItemById raw then |0x80000000 -> GetName
// -> drop the "{NN}" tag), but only the inventory panel is read and only the selected id is used.
bool resolveItemName(void* listbox, char* out, int cap) {
    if (!listbox || !out || cap < 2) return false;
    out[0] = 0;
    saferead::beginScope();
    const uintptr_t lb = reinterpret_cast<uintptr_t>(listbox);
    uint32_t panel = 0, vt = 0, sel = 0;
    if (!guikit::detail::rd32(lb, rmt::kCtlOff_ParentPanel, &panel) || !panel ||
        !saferead::probeRead(guikit::detail::SiteBase, reinterpret_cast<const void*>(panel), 4) ||
        !guikit::detail::rd32(panel, 0, &vt) || vt != kInventoryVtable) return false;
    if (!guikit::detail::rd32(panel, kPanelSelIdOff, &sel) || sel == 0 || sel == 0x7F000000u) return false;
    uint32_t gApp = 0, tbl = 0;
    if (!guikit::detail::rd32(gameaddr::AppGlobal, 0, &gApp) || !gApp || !guikit::detail::rd32(gApp, 4, &tbl) || !tbl) return false;
    auto clientGet = reinterpret_cast<uintptr_t(__thiscall*)(void*, uint32_t)>(gameaddr::ClientGetItemById);
    uintptr_t item = clientGet(reinterpret_cast<void*>(static_cast<uintptr_t>(tbl)), sel);
    if (!item) item = clientGet(reinterpret_cast<void*>(static_cast<uintptr_t>(tbl)), sel | 0x80000000u);
    uint32_t ivt = 0;
    if (!item || !guikit::detail::rd32(item, 0, &ivt) || !ivt) return false;
    int exo[2] = {0, 0};   // CExoString {char* p; int len}
    reinterpret_cast<uintptr_t(__thiscall*)(void*, void*, int)>(gameaddr::ClientItemGetName)(reinterpret_cast<void*>(item), exo, 0);
    if (!exo[0]) return false;
    bool ok = false;
    char tmp[256];
    if (saferead::readExo(0xE70118u, reinterpret_cast<uintptr_t>(exo), tmp, sizeof tmp, 255, false)) {
        const char* s = tmp;
        if (s[0] == '{') { const char* c = strchr(s, '}'); if (c) s = c + 1; }   // drop the engine's "{NN}" tag
        int k = 0;
        for (; s[k] && k < cap - 1; ++k) out[k] = (static_cast<unsigned char>(s[k]) >= 0x20 && static_cast<unsigned char>(s[k]) != 0x7F) ? s[k] : '.';
        out[k] = 0;
        ok = out[0] != 0;
    }
    reinterpret_cast<void(__thiscall*)(void*)>(gameaddr::ExoDtor)(exo);
    return ok;
}

bool itemTitleFn(void* listbox, char* out, int cap) { return resolveItemName(listbox, out, cap); }

void probeItemName(void* listbox, uintptr_t frame) {
    static int s_lines = 0, s_otherLines = 0;   // inventory lines and other-panel lines have separate caps (W3 review)
    static uint32_t s_lastPanel = 0, s_lastRow = 0, s_lastWin[kProbeWinWords] = {};
    if (!g_probeItemName || !listbox || (s_lines >= kProbeMaxLines && s_otherLines >= kProbeOtherMax) || !KpLog_HasCore()) return;
    saferead::beginScope();
    const uintptr_t lb = reinterpret_cast<uintptr_t>(listbox);
    uint32_t panel = 0, vt = 0;
    if (!guikit::detail::rd32(lb, rmt::kCtlOff_ParentPanel, &panel) || !panel ||
        !saferead::probeRead(guikit::detail::SiteBase, reinterpret_cast<const void*>(panel), 4) ||
        !guikit::detail::rd32(panel, 0, &vt)) {
        if (s_lastPanel != 0xFFFFFFFFu) { KP_LOG(chMain, "ITEMNAME probe: listbox=%p has no readable parent panel", listbox); ++s_lines; }
        s_lastPanel = 0xFFFFFFFFu;
        return;
    }
    const bool inv = (vt == kInventoryVtable);
    uint32_t win[kProbeWinWords] = {};
    for (int i = 0; i < kProbeWinWords; ++i) (void)guikit::detail::rd32(panel, kPanelWinOff + 4 * i, &win[i]);   // all panels: find the hover/selection words on equip and store too
    // P2a4 (R4): the detour sits at AddControls_2's entry, so the saved EBP in our frame should be OnControlEntered's EBP, whose [ebp+8] is the hovered row
    // and row+0x1D0 the item's SERVER id. Walk up to 3 frames, log each, take the first with a readable row and non-zero id.
    uint32_t rowId = 0, rowPtr = 0, rowEbp = 0; int rowLvl = -1; char chain[160] = {0}; int cl = 0;
    if (inv && frame) {
        uintptr_t f = frame;
        for (int lvl = 0; lvl < 3 && f; ++lvl) {
            uint32_t sv = 0, rp = 0, rid = 0;
            if (!guikit::detail::rd32(f, 0, &sv) || !sv) break;
            const bool rOk = guikit::detail::rd32(sv, 8, &rp) && rp && guikit::detail::rd32(rp, 0x1D0, &rid);
            if (cl < 120) cl += snprintf(chain + cl, sizeof chain - cl, "L%d ebp=%08x row=%08x id=%08x; ", lvl, sv, rOk ? rp : 0u, rOk ? rid : 0u);
            if (rowLvl < 0 && rOk && rid && rid != 0x7F000000u) { rowLvl = lvl; rowId = rid; rowPtr = rp; rowEbp = sv; }
            f = sv;
        }
    }
    bool changed = (panel != s_lastPanel) || (rowId != s_lastRow);
    s_lastRow = rowId;
    for (int i = 0; i < kProbeWinWords && !changed; ++i) changed = (win[i] != s_lastWin[i]);
    if (!changed) return;
    if (inv ? s_lines >= kProbeMaxLines : s_otherLines >= kProbeOtherMax) return;
    s_lastPanel = panel;
    for (int i = 0; i < kProbeWinWords; ++i) s_lastWin[i] = win[i];
    ++(inv ? s_lines : s_otherLines);
    if (!inv) {
        KP_LOG(chMain, "ITEMNAME probe: listbox=%p panel=%08x vtable=%08x (%s) | win[24C0..24DC]=%08x %08x %08x %08x %08x %08x %08x %08x",
               listbox, panel, vt, vt == 0x9a7624u ? "EQUIP" : vt == 0x9a701cu ? "OPTIONS" : vt == 0x99e6a4u ? "MODAL" : vt == 0x9a24dcu ? "JOURNAL" : "other panel", win[0], win[1], win[2], win[3], win[4], win[5], win[6], win[7]);
        return;
    }
    uint32_t sel = 0;
    (void)guikit::detail::rd32(panel, kPanelSelIdOff, &sel);
    // P2a2 (revised after the W3-B review): win[0] (+0x24C0) is NOT a hover index. It looks like the count of the panel's New-item id list
    // (ptr at +0x24BC, count +0x24C0, capacity +0x24C4: 0x10 then 0x20), which OnControlEntered appends to when a New row is hovered.
    // If so the LAST entry is the most recently hovered New item's id. Test that: id = list[count-1], resolve its name.
    uint32_t newCnt = win[0], newArr = 0, hovId = 0;
    (void)guikit::detail::rd32(panel, kPanelNewListOff, &newArr);
    bool hovOk = false;
    if (newArr && newCnt > 0 && newCnt < 4096 && guikit::detail::rd32(newArr, 4 * (newCnt - 1), &hovId) && hovId) hovOk = true;
    const bool selNone = (sel == 0 || sel == 0x7F000000u);
    const char* idSrc = selNone ? (hovOk ? "new-list-last" : "none") : "selected";
    if (selNone) sel = hovOk ? hovId : 0;
    if (rowId) { sel = rowId; idSrc = "caller-row"; }
    KP_LOG(chMain, "ITEMNAME probe: frame lvl=%d row=%08x rowEbp=%08x id=%08x | %s", rowLvl, rowPtr, rowEbp, rowId, chain);
    char name[96] = {0};
    const char* why = "";
    uintptr_t item = 0;
    uint32_t gApp = 0, tbl = 0, item14 = 0;
    if (sel == 0 || sel == 0x7F000000u) why = "no selection (0 or NoSelection)";
    else if (!guikit::detail::rd32(gameaddr::AppGlobal, 0, &gApp) || !gApp || !guikit::detail::rd32(gApp, 4, &tbl) || !tbl) why = "client item table unavailable";
    else {
        // P2a3 (R1/R2): the New-list value is the raw SERVER object id; client ids carry bit 31. Try raw, then sel|0x80000000.
        auto clientGet = reinterpret_cast<uintptr_t(__thiscall*)(void*, uint32_t)>(gameaddr::ClientGetItemById);
        item = clientGet(reinterpret_cast<void*>(static_cast<uintptr_t>(tbl)), sel);
        const char* form = "raw";
        if (!item) { item = clientGet(reinterpret_cast<void*>(static_cast<uintptr_t>(tbl)), sel | 0x80000000u); form = "or80000000"; }
        uint32_t srvTbl = 0, srvItem = 0;   // control: the server object table (R2's best resolver)
        if (guikit::detail::rd32(gApp, 8, &srvTbl) && srvTbl)
            srvItem = reinterpret_cast<uint32_t(__thiscall*)(void*, uint32_t)>(gameaddr::GetItemByObjectId)(reinterpret_cast<void*>(static_cast<uintptr_t>(srvTbl)), sel);
        KP_LOG(chMain, "ITEMNAME probe: resolve id=%08x clientForm=%s client=%08x server=%08x", sel, item ? form : "none", static_cast<unsigned>(item), srvItem);
        uint32_t ivt = 0;
        if (item) (void)guikit::detail::rd32(item, 0x14, &item14);   // ISF: item+0x14 is the creation-path id; must equal the id we looked up
        if (!item || !guikit::detail::rd32(item, 0, &ivt) || !ivt) { why = "id not found in the client item table"; item = 0; }
        else {
            int exo[2] = {0, 0};   // CExoString {char* p; int len}
            reinterpret_cast<uintptr_t(__thiscall*)(void*, void*, int)>(gameaddr::ClientItemGetName)(reinterpret_cast<void*>(item), exo, 0);
            if (exo[0]) {
                char tmp[256];
                if (saferead::readExo(0xE70118u, reinterpret_cast<uintptr_t>(exo), tmp, sizeof tmp, 255, false)) {
                    const char* s = tmp;
                    if (s[0] == '{') { const char* c = strchr(s, '}'); if (c) s = c + 1; }   // drop the engine's "{NN}" tag
                    int k = 0;
                    for (; s[k] && k < static_cast<int>(sizeof name) - 1; ++k) name[k] = (static_cast<unsigned char>(s[k]) >= 0x20 && static_cast<unsigned char>(s[k]) < 0x7F && s[k] != '"') ? s[k] : '.';
                    name[k] = 0;
                    if (!name[0]) why = "name empty";
                } else why = "readExo failed";
                reinterpret_cast<void(__thiscall*)(void*)>(gameaddr::ExoDtor)(exo);
            } else why = "GetName returned no text";
        }
    }
    KP_LOG(chMain, "ITEMNAME probe: inv panel=%08x id=%08x via=%s newcnt=%u cap=%u arr=%08x item=%08x i14=%08x name=\"%s\" %s | win[24C0..24DC]=%08x %08x %08x %08x %08x %08x %08x %08x",
           panel, sel, idSrc, newCnt, win[1], newArr, static_cast<unsigned>(item), item14, name, why, win[0], win[1], win[2], win[3], win[4], win[5], win[6], win[7]);
}

extern "C" int __cdecl RmtAddControls2(void* listbox, void** pControls, int count,
                                      int bAutoDelete, int bRestoreExtents, int bVariableItemHeights,
                                      uintptr_t origFnAddr) {
    if (!g_richDescBox || !listbox || !origFnAddr) {
        return 0;
    }
    probeItemName(listbox, reinterpret_cast<uintptr_t>(__builtin_frame_address(0)));

    auto origFn = reinterpret_cast<rmt::FnAddControls2>(origFnAddr);
    const bool ok = rmt::RichDescBox::populate(listbox, pControls, count,
                                               bAutoDelete, bRestoreExtents, bVariableItemHeights,
                                               origFn);
    if (ok) {
        ++g_descPopulateCount;
        if (KpLog_HasCore() && g_verbose) {
            if (g_descPopulateCount <= 20) {
                KP_LOGV(chMain, KPLOG_DEBUG, "RichDescBox populated listbox=%p popCount=%u",
                        listbox, g_descPopulateCount);
            }
        }
        return 1;
    }
    return 0;
}

extern "C" int __cdecl RmtSetLabelBonusPenaltyColour(void* pLabel, int delta) {
    if (!g_statHierarchy || !pLabel) {
        return 0;
    }

    const float* const col = (delta > 0) ? rmt::kBonusAmber
                           : (delta < 0) ? rmt::kPenaltyRed
                                         : rmt::kCream;
    if (styleLabelColor(reinterpret_cast<uintptr_t>(pLabel), col)) {
        ++g_statStyleCount;
        return 1;
    }
    return 0;
}

extern "C" void __cdecl RmtSetStats(void* panel) {
    if (!g_statHierarchy || !panel) {
        return;
    }

    const uintptr_t base = reinterpret_cast<uintptr_t>(panel);
    uint32_t styled = 0;

    for (size_t off : kCharCreamOffsets) {
        if (styleLabelColor(base + off, rmt::kCream, true)) {
            ++styled;
        }
    }
    for (size_t off : kCharTealDimOffsets) {
        if (styleLabelColor(base + off, rmt::kTealDim, true)) {
            ++styled;
        }
    }

    if (styled > 0) {
        g_statStyleCount += styled;
        if (KpLog_HasCore()) {
            if (g_verbose) {
                if (g_lastStatsPanel != base || g_lastStatsStyled != styled) {
                    g_lastStatsPanel = base;
                    g_lastStatsStyled = styled;
                    KP_LOGV(chMain, KPLOG_DEBUG, "RmtSetStats styled %u labels on character panel=%p",
                            styled, panel);
                }
            } else if (!g_statsLogged) {
                g_statsLogged = true;
                g_lastStatsPanel = base;
                g_lastStatsStyled = styled;
                KP_LOGV(chMain, KPLOG_DEBUG, "RmtSetStats styled %u labels on character panel=%p",
                        styled, panel);
            }
        }
    }
}

extern "C" void __cdecl RmtStyleInitControl(void* pControl, const char* tagStr) {
    if (!g_mcReg.registered) pagePoll();

    if (!g_statHierarchy || !pControl || !tagStr || !*tagStr) {
        return;
    }

    const rmt::MenuLabelRole role = rmt::classifyMenuLabelTag(tagStr);
    if (role == rmt::MenuLabelRole::None) {
        return;
    }

    const float* const col = (role == rmt::MenuLabelRole::TitleOrValue) ? rmt::kCream : rmt::kTealDim;
    if (styleLabelColor(reinterpret_cast<uintptr_t>(pControl), col, true)) {
        ++g_statStyleCount;
        if (KpLog_HasCore() && g_verbose) {
            if (g_initControlLogs < 30) {
                ++g_initControlLogs;
                KP_LOGV(chMain, KPLOG_DEBUG, "RmtStyleInitControl tag=%s role=%u ctl=%p",
                        tagStr, static_cast<unsigned>(role), pControl);
            }
        }
    }
}

extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    const char* const iniPath = ".\\rich_menu_typography.ini";
    g_inlineColoring = GetPrivateProfileIntA("General", "InlineColoring", 1, iniPath) != 0;
    g_richDescBox = GetPrivateProfileIntA("General", "RichDescBox", 1, iniPath) != 0;
    g_statHierarchy = GetPrivateProfileIntA("General", "StatHierarchy", 1, iniPath) != 0;
    g_verbose = GetPrivateProfileIntA("General", "Verbose", 0, iniPath) != 0;
    g_probeItemName = GetPrivateProfileIntA("General", "ProbeItemName", 0, iniPath) != 0;
    rmt::semanticColors() = GetPrivateProfileIntA("General", "SemanticColors", 1, iniPath) != 0;
    rmt::dividerMode() = static_cast<int>(GetPrivateProfileIntA("General", "DividerMode", 1, iniPath));
    if (rmt::dividerMode() < 0) rmt::dividerMode() = 0; else if (rmt::dividerMode() > 2) rmt::dividerMode() = 2;
    g_itemNameTitle = GetPrivateProfileIntA("General", "ItemNameTitle", 1, iniPath) == 1;   // exact 1 only, same range as the page row (0..1)
    rmt::g_itemTitleFn = g_itemNameTitle ? itemTitleFn : nullptr;
    const bool probeRmt = GetPrivateProfileIntA("General", "ProbeRmt", 0, iniPath) != 0;
    rmt::g_probeLog = probeRmt ? probeLogForwarder : nullptr;
    rmt::g_probeEveryPopulate = probeRmt;  // log every populate only while probing (doc 53)

    static char s_bodyFontBuf[32] = {0};
    GetPrivateProfileStringA("General", "BodyFont", rmt::kFontDesc, s_bodyFontBuf, sizeof(s_bodyFontBuf), iniPath);
    rmt::setBodyFont(s_bodyFontBuf);

    static char s_rulesFontBuf[32] = {0};
    GetPrivateProfileStringA("General", "RulesFont", rmt::kFontDesc, s_rulesFontBuf, sizeof(s_rulesFontBuf), iniPath);
    rmt::setRulesFont(s_rulesFontBuf);

    if (KpLog_HasCore()) {
        KP_LOG(chMain, "INIT rich-menu-typography inlineColoring=%d richDescBox=%d statHierarchy=%d bodyFont=%s rulesFont=%s verbose=%d",
               g_inlineColoring ? 1 : 0, g_richDescBox ? 1 : 0, g_statHierarchy ? 1 : 0, rmt::getBodyFont(), rmt::getRulesFont(), g_verbose ? 1 : 0);
    }

    pageInit(api);
}

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID /*reserved*/) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
    }
    return TRUE;
}
#endif
