// RichMenuTypography.cpp -- KPM DLL entry and per-glyph inline color, description listbox & static menu hierarchy hooks (Doc 53 RMT1..RMT3).
#include "RichMenuTypography.h"
#include "RichDescBox.h"
#include "RichTextClassifier.h"
#include <windows.h>
#include <cstdint>
#include <cstring>
#include "KpLog.h"

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
#include "ModConfigApi.h"
#include "ProviderClient.h"
#include "ModConfigDecl.h"
#include "RichMenuTypographyPage.h"

namespace {
bool g_inlineColoring = true;
bool g_richDescBox = true;
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

int32_t pageOnChange(void* /*user*/, int /*row*/, const char* id, int32_t requested, int /*dir*/) {
    if (id && strcmp(id, rmtpage::kRowReset) == 0 && requested != 0) {
        g_live.inlineColoring = true;
        g_live.richDesc = true;
        g_live.stats = true;
        g_live.fontIdx = 0;
        g_live.rulesFontIdx = 0;
        g_inlineColoring = true;
        g_richDescBox = true;
        g_statHierarchy = true;
        rmt::setBodyFont(rmtpage::fontName(0));
        rmt::setRulesFont(rmtpage::fontName(0));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "InlineColoring", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "RichDescBox", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "StatHierarchy", "1");
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "BodyFont", rmtpage::fontName(0));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "RulesFont", rmtpage::fontName(0));
        g_page.setValue(rmtpage::kRowInline, 1);
        g_page.setValue(rmtpage::kRowRichDesc, 1);
        g_page.setValue(rmtpage::kRowStats, 1);
        g_page.setValue(rmtpage::kRowFont, 0);
        g_page.setValue(rmtpage::kRowRulesFont, 0);
        g_page.setValue(rmtpage::kRowReset, 0);
        if (KpLog_HasCore()) KP_LOG(chMain, "PAGE reset to factory defaults");
        return 0;
    }
    const int32_t fin = rmtpage::applyRow(g_live, id, requested);
    g_inlineColoring = g_live.inlineColoring;
    g_richDescBox = g_live.richDesc;
    g_statHierarchy = g_live.stats;
    if (id && strcmp(id, rmtpage::kRowFont) == 0) {
        rmt::setBodyFont(rmtpage::fontName(g_live.fontIdx));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "BodyFont", rmtpage::fontName(g_live.fontIdx));
        if (g_mcMenu.ok() && g_mcMenu.p && g_mcMenu.p->SetDesc) {
            g_mcMenu.p->SetDesc(g_page.page, "Body Font Preview:\nThe quick brown fox jumps over the lazy dog.\nDamage: 3-13 (+1-6 Energy)");
        }
    }
    if (id && strcmp(id, rmtpage::kRowRulesFont) == 0) {
        rmt::setRulesFont(rmtpage::fontName(g_live.rulesFontIdx));
        iniw::updateFile(rmtpage::kIniFile, rmtpage::kSection, "RulesFont", rmtpage::fontName(g_live.rulesFontIdx));
        if (g_mcMenu.ok() && g_mcMenu.p && g_mcMenu.p->SetDesc) {
            g_mcMenu.p->SetDesc(g_page.page, "Rules Font Preview:\nFeats Required: Weapon Proficiency\nDefense Bonus: +2\nDamage: 3-13");
        }
    }
    if (g_pageLogs < 20) {
        ++g_pageLogs;
        if (KpLog_HasCore()) {
            KP_LOG(chMain, "PAGE row=%s req=%d final=%d", id ? id : "", static_cast<int>(requested), static_cast<int>(fin));
        }
    }
    return fin;
}

void pageOnEnter(void* /*user*/) {
    g_page.setValue(rmtpage::kRowFont, rmtpage::fontIndexFor(rmt::getBodyFont()));
    g_page.setValue(rmtpage::kRowRulesFont, rmtpage::fontIndexFor(rmt::getRulesFont()));
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
    g_mcEntry = g_page.entry(rmtpage::kEntryId);

    g_live.inlineColoring = g_inlineColoring;
    g_live.richDesc = g_richDescBox;
    g_live.stats = g_statHierarchy;
    g_live.fontIdx = rmtpage::fontIndexFor(rmt::getBodyFont());
    g_live.rulesFontIdx = rmtpage::fontIndexFor(rmt::getRulesFont());
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

inline bool styleLabelColor(uintptr_t ctl, const float rgb[3]) noexcept {
    if (!ctl || !rgb) return false;
    uint32_t vt = 0;
    const uint32_t wantLabelVt = guikit::detail::vtab(&K2AddrTable::LabelVtable);
    if (!wantLabelVt || !guikit::detail::rd32(ctl, 0, &vt) || vt != wantLabelVt) {
        return false;
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
void setVerbose(bool verbose) noexcept { g_verbose = verbose; }
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

extern "C" int __cdecl RmtAddControls2(void* listbox, void** pControls, int count,
                                      int bAutoDelete, int bRestoreExtents, int bVariableItemHeights,
                                      uintptr_t origFnAddr) {
    if (!g_richDescBox || !listbox || !origFnAddr) {
        return 0;
    }

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
        if (styleLabelColor(base + off, rmt::kCream)) {
            ++styled;
        }
    }
    for (size_t off : kCharTealDimOffsets) {
        if (styleLabelColor(base + off, rmt::kTealDim)) {
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
    if (styleLabelColor(reinterpret_cast<uintptr_t>(pControl), col)) {
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
