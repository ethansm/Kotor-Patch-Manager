#pragma once

#ifndef _WIN32
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#endif

#include "GameAddr.h"
#include "GuiKit.h"
#include "RichTextClassifier.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef __thiscall
#if defined(_MSC_VER) || defined(__MINGW32__)
#define __thiscall __thiscall
#elif defined(__i386__)
#define __thiscall __attribute__((thiscall))
#else
#define __thiscall
#endif
#endif

namespace rmt {

using FnAddControls2 = void(__thiscall*)(void* listbox, void** pControls, int count,
                                         int bAutoDelete, int bRestoreExtents, int bVariableItemHeights);

// K5 vertical-centering vanishing prevention: NEVER use 17 or 18
constexpr unsigned kAlignTopLeft = 9u;
constexpr int kDividerHeight = 2;
constexpr const char* kDividerFillResRef = "uibit_fill_16g";
constexpr size_t kMaxPooledRows = 6;
constexpr size_t kMaxListBoxPools = 32;

// ListBox & Label offsets
constexpr int kCtlOff_ParentPanel = 0x38;
constexpr int kLabelOff_TextParams = 0xF0;
constexpr int kLabelOff_Renderer = 0xEC;
constexpr int kTextVtOff_GetFormattedHeight = 0x08; // slot 2 of CSWGuiText vtable at label + Label_Text
constexpr int kListBoxOff_ClientW = 0x2A4;
constexpr int kListBoxOff_ClientH = 0x2A8;
constexpr int kListBoxOff_Flags = 0x2CC;
constexpr int kListBoxOff_Padding = 0x2D0;
constexpr int kListBoxOff_TopItem = 0x2D8;

struct LastPopulateInfo {
    uintptr_t listbox = 0;
    int rowCount = 0;
    int rowHeights[kMaxPooledRows] = {};
    unsigned rowAligns[kMaxPooledRows] = {};
    char rowFonts[kMaxPooledRows][20] = {};
    char rowTexts[kMaxPooledRows][256] = {};
    bool rowIsDivider[kMaxPooledRows] = {};
    float rowColors[kMaxPooledRows][3] = {};
};

struct PoolEntry {
    uintptr_t listbox = 0;
    uintptr_t parentPanel = 0;
    uintptr_t labels[kMaxPooledRows] = {};
    uint32_t lastUse = 0;
};

inline LastPopulateInfo s_lastPopulateInfo = {};
inline PoolEntry s_listBoxPools[kMaxListBoxPools] = {};
inline uint32_t s_useCounter = 0;
inline uint8_t s_dummyRenderer[0x80] = {0};
inline const char* s_bodyFont = kFontDesc;
inline const char* s_rulesFont = kFontDesc;
inline int (*g_measureTextHeight)(uintptr_t ctl, const char* text, int width, const char* font) = nullptr;

inline const char* getBodyFont() noexcept {
    return s_bodyFont ? s_bodyFont : kFontDesc;
}

inline void setBodyFont(const char* fontResRef) noexcept {
    if (fontResRef && *fontResRef) {
        s_bodyFont = fontResRef;
    } else {
        s_bodyFont = kFontDesc;
    }
}

inline const char* getRulesFont() noexcept {
    return s_rulesFont ? s_rulesFont : kFontDesc;
}

inline void setRulesFont(const char* fontResRef) noexcept {
    if (fontResRef && *fontResRef) {
        s_rulesFont = fontResRef;
    } else {
        s_rulesFont = kFontDesc;
    }
}

inline const LastPopulateInfo& getLastPopulateInfo() noexcept {
    return s_lastPopulateInfo;
}

inline void resetPool() noexcept {
    s_lastPopulateInfo = LastPopulateInfo{};
    for (size_t i = 0; i < kMaxListBoxPools; ++i) {
        s_listBoxPools[i] = PoolEntry{};
    }
    s_useCounter = 0;
}

inline float getEngineTextScalar() noexcept {
#ifndef GUIKIT_HOST
    float liveScalar = 0.0f;
    if (saferead::readAt<float>(guikit::detail::SiteBase, gameaddr::TextScalar, 0, &liveScalar) &&
        liveScalar >= 0.5f && liveScalar <= 8.0f) {
        return liveScalar;
    }
    int screenW = 0;
    if (saferead::readAt<int>(guikit::detail::SiteBase, gameaddr::ScreenW, 0, &screenW) && screenW > 0) {
        if (screenW <= 1022) return 1.0f;
        return 1.0f + 0.5f * static_cast<float>((screenW - 1022) >> 10);
    }
#endif
    return 1.0f;
}

inline int measureTextHeight(uintptr_t ctl, const char* text, int width, const char* font = nullptr) noexcept {
    if (g_measureTextHeight) {
        return g_measureTextHeight(ctl, text, width, font);
    }
    if (ctl && guikit::detail::textObjOk(ctl)) {
#ifndef GUIKIT_HOST
        const uintptr_t textObj = ctl + guikit::Label_Text;
        const float scalar = getEngineTextScalar();
        uint32_t vt = 0;
        if (guikit::detail::rd32(textObj, 0, &vt) && vt) {
            uint32_t fn = 0;
            if (guikit::detail::rd32(vt, kTextVtOff_GetFormattedHeight, &fn) && fn) {
                int physicalH = reinterpret_cast<int(__thiscall*)(void*)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(textObj));
                if (physicalH > 0 && scalar > 0.01f) {
                    return static_cast<int>(static_cast<float>(physicalH) / scalar + 0.99f);
                }
            }
        }
#endif
    }
    if (!text || !*text) return 0;

    int lineHeight = 14;
    if (font) {
        if (strcmp(font, kFontSub) == 0 || strstr(font, "10x10") != nullptr) {
            lineHeight = 8;
        } else if (strcmp(font, kFontTitle) == 0 || (strstr(font, "16x16") != nullptr && strstr(font, "dialog") != nullptr)) {
            lineHeight = 12;
        } else if (strcmp(font, kFontDesc) == 0 || strstr(font, "fnt_d") != nullptr || strstr(font, "fnt_") != nullptr) {
            lineHeight = 14;
        }
    }

    const int charW = (lineHeight >= 14) ? 7 : (lineHeight >= 12 ? 6 : 5);
    const int charsPerLine = (width > 35) ? (width / charW) : 28;
    const int halfLine = (lineHeight + 1) / 2;
    int totalHeight = 0;
    const char* p = text;
    while (*p) {
        const char* start = p;
        while (*p && *p != '\n') ++p;
        const int plen = static_cast<int>(p - start);
        if (plen == 0) {
            totalHeight += halfLine;
        } else {
            const int lineCount = (plen + charsPerLine - 1) / charsPerLine;
            totalHeight += lineCount * lineHeight;
        }
        if (*p == '\n') ++p;
    }
    if (totalHeight < lineHeight) totalHeight = lineHeight;
    return totalHeight;
}

inline PoolEntry* findOrAllocPool(uintptr_t listbox) noexcept {
    if (!listbox) return nullptr;
    ++s_useCounter;

    uintptr_t curParent = 0;
    (void)guikit::detail::rd<uintptr_t>(listbox, kCtlOff_ParentPanel, &curParent);

    // Look for existing entry
    for (size_t i = 0; i < kMaxListBoxPools; ++i) {
        if (s_listBoxPools[i].listbox == listbox) {
            if (s_listBoxPools[i].parentPanel != 0 && curParent != 0 &&
                s_listBoxPools[i].parentPanel != curParent) {
                memset(s_listBoxPools[i].labels, 0, sizeof(s_listBoxPools[i].labels));
            }
            s_listBoxPools[i].parentPanel = curParent;
            s_listBoxPools[i].lastUse = s_useCounter;
            return &s_listBoxPools[i];
        }
    }

    // Look for an empty slot
    for (size_t i = 0; i < kMaxListBoxPools; ++i) {
        if (s_listBoxPools[i].listbox == 0) {
            s_listBoxPools[i].listbox = listbox;
            s_listBoxPools[i].parentPanel = curParent;
            s_listBoxPools[i].lastUse = s_useCounter;
            memset(s_listBoxPools[i].labels, 0, sizeof(s_listBoxPools[i].labels));
            return &s_listBoxPools[i];
        }
    }

    // LRU eviction
    size_t lruIdx = 0;
    uint32_t oldest = s_listBoxPools[0].lastUse;
    for (size_t i = 1; i < kMaxListBoxPools; ++i) {
        if (s_listBoxPools[i].lastUse < oldest) {
            oldest = s_listBoxPools[i].lastUse;
            lruIdx = i;
        }
    }
    s_listBoxPools[lruIdx].listbox = listbox;
    s_listBoxPools[lruIdx].parentPanel = curParent;
    s_listBoxPools[lruIdx].lastUse = s_useCounter;
    memset(s_listBoxPools[lruIdx].labels, 0, sizeof(s_listBoxPools[lruIdx].labels));
    return &s_listBoxPools[lruIdx];
}

inline uintptr_t ensurePooledLabel(PoolEntry& slot, size_t idx) noexcept {
    if (idx >= kMaxPooledRows) return 0;
    if (slot.labels[idx] != 0) {
        uint32_t vt = 0;
        if (guikit::detail::rd32(slot.labels[idx], 0, &vt) && vt == guikit::LabelVtable) {
            return slot.labels[idx];
        }
    }

    void* mem = nullptr;
#ifdef GUIKIT_HOST
    if (guikit::g_gk.alloc) {
        mem = guikit::g_gk.alloc(static_cast<unsigned>(guikit::LabelSize));
    } else {
        mem = malloc(guikit::LabelSize);
    }
#else
    if (guikit::g_gk.alloc) {
        mem = guikit::g_gk.alloc(static_cast<unsigned>(guikit::LabelSize));
    }
    if (!mem) {
        mem = malloc(guikit::LabelSize);
    }
#endif
    if (!mem) return 0;

    memset(mem, 0, guikit::LabelSize);
    if (guikit::g_gk.labelCtor) {
        guikit::g_gk.labelCtor(mem);
    }

    uintptr_t c = reinterpret_cast<uintptr_t>(mem);
    uint32_t vt = 0;
    if (!guikit::detail::rd32(c, 0, &vt) || vt == 0) {
        *reinterpret_cast<uint32_t*>(c + 0) = guikit::LabelVtable;
    }
    if (!guikit::detail::rd32(c, guikit::Label_Border, &vt) || vt == 0) {
        *reinterpret_cast<uint32_t*>(c + guikit::Label_Border) = guikit::BorderVtable;
    }
    if (!guikit::detail::rd32(c, guikit::Label_Text, &vt) || vt == 0) {
        *reinterpret_cast<uint32_t*>(c + guikit::Label_Text) = guikit::TextVtable;
    }

    uint32_t rend = 0;
    if (!guikit::detail::rd32(c, kLabelOff_Renderer, &rend) || rend == 0) {
        const uint32_t dummy = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s_dummyRenderer));
        *reinterpret_cast<uint32_t*>(c + kLabelOff_Renderer) = dummy;
    }

    slot.labels[idx] = c;
    return c;
}

struct RichDescBox {
    static inline bool populate(void* listbox, void** pControls, int count,
                                int bAutoDelete, int bRestoreExtents, int bVariableItemHeights,
                                FnAddControls2 origAddControls2) noexcept {
        static thread_local bool s_inPopulate = false;
        if (s_inPopulate) return false;
        (void)bRestoreExtents;

        if (!listbox || !pControls || !origAddControls2) return false;
        if (count != 1 || bAutoDelete != 0 || bVariableItemHeights != 0) return false;

        const uintptr_t srcCtl = reinterpret_cast<uintptr_t>(pControls[0]);
        if (!srcCtl) return false;

        uint32_t vt = 0;
        if (!guikit::detail::rd32(srcCtl, 0, &vt) || vt != guikit::LabelVtable) {
            return false;
        }

        const char* rawText = nullptr;
        (void)guikit::detail::rd<const char*>(srcCtl, kLabelOff_TextParams, &rawText);
        if (!rawText || !saferead::probeRead(guikit::detail::SiteBase, rawText, 1)) {
            rawText = nullptr;
            uint32_t rend32 = 0;
            if (guikit::detail::rd32(srcCtl, kLabelOff_Renderer, &rend32) && rend32) {
                const uintptr_t rend = static_cast<uintptr_t>(rend32);
                if (saferead::probeRead(guikit::detail::SiteBase, reinterpret_cast<const void*>(rend), sizeof(void*))) {
                    (void)guikit::detail::rd<const char*>(rend, kStrOff_PszStr, &rawText);
                }
            }
        }
        if (!rawText || !saferead::probeRead(guikit::detail::SiteBase, rawText, 1) || !*rawText) {
            return false;
        }

        DescSections sec{};
        if (!splitDescSections(rawText, &sec)) {
            return false;
        }

        int clientW = 0;
        uint8_t pad = 0;
        (void)guikit::detail::rdInt(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_ClientW, &clientW);
        (void)guikit::detail::rd<uint8_t>(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_Padding, &pad);
        int lbItemW = clientW - 2 * static_cast<int>(pad);
        int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
        (void)guikit::getExtent(srcCtl, &srcX, &srcY, &srcW, &srcH);
        int w = lbItemW;
        if (srcW > 0 && srcW < w) {
            w = srcW;
        }
        if (w <= 0) {
            w = (srcW > 0) ? srcW : 240;
        }

        int clientH = 0;
        (void)guikit::detail::rdInt(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_ClientH, &clientH);
        if (clientH <= 0) {
            (void)guikit::getExtent(reinterpret_cast<uintptr_t>(listbox), nullptr, nullptr, nullptr, &clientH);
        }
        if (clientH <= 0) clientH = 140;

        const float textScalar = getEngineTextScalar();
        (void)textScalar;

        PoolEntry* pool = findOrAllocPool(reinterpret_cast<uintptr_t>(listbox));
        if (!pool) return false;

        s_lastPopulateInfo = LastPopulateInfo{};
        void* outControls[kMaxPooledRows] = {nullptr};
        int outCount = 0;

        auto measureRowH = [&](uintptr_t lbl, const char* text, const char* font) -> int {
            (void)guikit::setRect(lbl, srcX, srcY, w, 32);
            (void)guikit::setFont(lbl, font);
            (void)guikit::setAlign(lbl, kAlignTopLeft);
            (void)guikit::setText(lbl, text);

            int finalH = 0;
#ifndef GUIKIT_HOST
            if (guikit::detail::textObjOk(lbl)) {
                const uintptr_t textObj = lbl + guikit::Label_Text;
                uint32_t vt = 0;
                if (guikit::detail::rd32(textObj, 0, &vt) && vt) {
                    uint32_t fn = 0;
                    if (guikit::detail::rd32(vt, kTextVtOff_GetFormattedHeight, &fn) && fn) {
                        int physicalH = reinterpret_cast<int(__thiscall*)(void*)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(textObj));
                        if (physicalH > 0 && textScalar > 0.01f) {
                            finalH = static_cast<int>(static_cast<float>(physicalH) / textScalar + 0.99f);
                        }
                    }
                }
            }
#endif
            if (finalH <= 0) {
                finalH = measureTextHeight(lbl, text, w, font);
            }
            return finalH;
        };

        float origCol[3] = { kTealText[0], kTealText[1], kTealText[2] };
        (void)guikit::getTextColor(srcCtl, origCol);
        const bool isTeal = isDefaultTeal(origCol[0], origCol[1], origCol[2]);

        const float* titleCol = isTeal ? kCream : ((origCol[0] > 0.5f && origCol[1] > 0.4f) ? kCream : origCol);
        const float subCol[3] = { origCol[0] * 0.75f, origCol[1] * 0.75f, origCol[2] * 0.75f };
        const float* rulesCol = isTeal ? kTealText : origCol;
        const float* bodyCol  = isTeal ? kTealText : origCol;

        const bool hasTitle    = (sec.title[0] != '\0');
        const bool hasSubtitle = (sec.subtitle[0] != '\0' && strcmp(sec.subtitle, sec.title) != 0);
        const bool hasRules    = (sec.rules[0] != '\0');
        const bool hasBody     = (sec.body[0] != '\0');

        uintptr_t lbls[4] = {0};
        int titleH = 0, subH = 0, rulesH = 0, bodyH = 0;
        int activeCount = 0;

        if (hasTitle) {
            lbls[activeCount] = ensurePooledLabel(*pool, static_cast<size_t>(activeCount));
            if (lbls[activeCount]) {
                titleH = measureRowH(lbls[activeCount], sec.title, kFontTitle);
                ++activeCount;
            }
        }
        if (hasSubtitle) {
            lbls[activeCount] = ensurePooledLabel(*pool, static_cast<size_t>(activeCount));
            if (lbls[activeCount]) {
                subH = measureRowH(lbls[activeCount], sec.subtitle, kFontSub);
                ++activeCount;
            }
        }
        if (hasRules) {
            lbls[activeCount] = ensurePooledLabel(*pool, static_cast<size_t>(activeCount));
            if (lbls[activeCount]) {
                rulesH = measureRowH(lbls[activeCount], sec.rules, getRulesFont());
                ++activeCount;
            }
        }
        if (hasBody) {
            lbls[activeCount] = ensurePooledLabel(*pool, static_cast<size_t>(activeCount));
            if (lbls[activeCount]) {
                bodyH = measureRowH(lbls[activeCount], sec.body, getBodyFont());
                ++activeCount;
            }
        }

        const int naturalSum = titleH + subH + rulesH + bodyH;

        int titleGap = 0, subGap = 0, rulesGap = 0;
        const int desTitleGap = (hasTitle && (hasSubtitle || hasRules || hasBody)) ? 4 : 0;
        const int desSubGap   = (hasSubtitle && (hasRules || hasBody)) ? 3 : 0;
        const int desRulesGap = (hasRules && hasBody) ? 4 : 0;
        const int numGaps = (desTitleGap > 0 ? 1 : 0) + (desSubGap > 0 ? 1 : 0) + (desRulesGap > 0 ? 1 : 0);

        const int padTotal = (activeCount > 1) ? (activeCount - 1) * static_cast<int>(pad) : 0;
        const int effectiveLimit = (clientH > padTotal) ? (clientH - padTotal) : clientH;

        if (numGaps > 0) {
            const int totalDesiredGaps = desTitleGap + desSubGap + desRulesGap;
            const int remainingSpace = effectiveLimit - naturalSum;

            if (remainingSpace >= totalDesiredGaps) {
                // Ample headroom: fit comfortably on one page with full gaps
                titleGap = desTitleGap;
                subGap   = desSubGap;
                rulesGap = desRulesGap;
            } else if (remainingSpace > 0) {
                // Natural text fits on one page, but full gaps would trigger a scrollbar!
                // Compact gaps to keep total height <= effectiveLimit so no scrollbar appears.
                int budget = remainingSpace;
                if (desRulesGap > 0 && budget > 0) {
                    rulesGap = std::min(desRulesGap, std::max(1, budget / numGaps));
                    budget -= rulesGap;
                }
                if (desTitleGap > 0 && budget > 0) {
                    titleGap = std::min(desTitleGap, std::max(1, budget / numGaps));
                    budget -= titleGap;
                }
                if (desSubGap > 0 && budget > 0) {
                    subGap = std::min(desSubGap, std::max(1, budget));
                    budget -= subGap;
                }
                while (naturalSum + titleGap + subGap + rulesGap > effectiveLimit) {
                    if (rulesGap > 0) --rulesGap;
                    else if (titleGap > 0) --titleGap;
                    else if (subGap > 0) --subGap;
                    else break;
                }
            } else {
                // Natural text genuinely exceeds effectiveLimit (multi-page item)
                // Use standard comfortable gaps for scrolling
                titleGap = desTitleGap;
                subGap   = desSubGap;
                rulesGap = desRulesGap;
            }
        }

        auto applyTextRow = [&](uintptr_t lbl, const char* text, const char* font, const float* col, int rowH, int gap) {
            if (!lbl || outCount >= static_cast<int>(kMaxPooledRows)) return;
            const size_t rowIdx = static_cast<size_t>(outCount);

            (void)guikit::setFill(lbl, nullptr);
            (void)guikit::detail::wr<uint32_t>(lbl + guikit::Label_Border, guikit::Border_FillStyle, 0);

            const int finalH = rowH + gap;

            (void)guikit::setRect(lbl, srcX, srcY, w, finalH);
            (void)guikit::setFont(lbl, font);
            (void)guikit::setAlign(lbl, kAlignTopLeft);
            (void)guikit::setTextColor(lbl, col[0], col[1], col[2]);
            (void)guikit::setText(lbl, text);

            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_X, srcX);
            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_Y, srcY);
            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_W, w);
            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_H, finalH);

            (void)guikit::setVisible(lbl, true);
            (void)guikit::setClickable(lbl, false);

            s_lastPopulateInfo.rowHeights[rowIdx] = finalH;
            s_lastPopulateInfo.rowAligns[rowIdx] = kAlignTopLeft;
            strncpy(s_lastPopulateInfo.rowFonts[rowIdx], font, sizeof(s_lastPopulateInfo.rowFonts[rowIdx]) - 1);
            strncpy(s_lastPopulateInfo.rowTexts[rowIdx], text, sizeof(s_lastPopulateInfo.rowTexts[rowIdx]) - 1);
            s_lastPopulateInfo.rowIsDivider[rowIdx] = false;
            s_lastPopulateInfo.rowColors[rowIdx][0] = col[0];
            s_lastPopulateInfo.rowColors[rowIdx][1] = col[1];
            s_lastPopulateInfo.rowColors[rowIdx][2] = col[2];

            outControls[outCount++] = reinterpret_cast<void*>(lbl);
        };

        int emitIdx = 0;
        if (hasTitle && lbls[emitIdx]) {
            applyTextRow(lbls[emitIdx++], sec.title, kFontTitle, titleCol, titleH, titleGap);
        }
        if (hasSubtitle && lbls[emitIdx]) {
            applyTextRow(lbls[emitIdx++], sec.subtitle, kFontSub, isTeal ? kTealDim : subCol, subH, subGap);
        }
        if (hasRules && lbls[emitIdx]) {
            applyTextRow(lbls[emitIdx++], sec.rules, getRulesFont(), rulesCol, rulesH, rulesGap);
        }
        if (hasBody && lbls[emitIdx]) {
            applyTextRow(lbls[emitIdx++], sec.body, getBodyFont(), bodyCol, bodyH, 0);
        }

        if (outCount == 0) return false;

        s_lastPopulateInfo.listbox = reinterpret_cast<uintptr_t>(listbox);
        s_lastPopulateInfo.rowCount = outCount;

        (void)guikit::detail::wr<uint16_t>(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_TopItem, 0);

        s_inPopulate = true;
        origAddControls2(listbox, outControls, outCount, /*bAutoDelete=*/0, /*bRestoreExtents=*/0, /*bVariableItemHeights=*/1);
        s_inPopulate = false;

        (void)guikit::detail::wr<uint16_t>(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_TopItem, 0);

        return true;
    }
};

} // namespace rmt
