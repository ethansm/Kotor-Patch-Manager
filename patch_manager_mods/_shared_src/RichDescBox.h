#pragma once

#ifndef _WIN32
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#endif

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

inline int measureTextHeight(uintptr_t ctl, const char* text, int width, const char* font = nullptr) noexcept {
    if (g_measureTextHeight) {
        return g_measureTextHeight(ctl, text, width, font);
    }
    if (ctl && guikit::detail::textObjOk(ctl)) {
        const uintptr_t textObj = ctl + guikit::Label_Text;
        uint32_t vt = 0;
        if (guikit::detail::rd32(textObj, 0, &vt) && vt) {
            uint32_t fn = 0;
            if (guikit::detail::rd32(vt, kTextVtOff_GetFormattedHeight, &fn) && fn) {
                int h = reinterpret_cast<int(__thiscall*)(void*)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(textObj));
                if (h > 0) return h;
            }
        }
    }
    if (!text || !*text) return 0;

    int lineHeight = 14;
    if (font) {
        if (strcmp(font, kFontSub) == 0 || strstr(font, "10x10") != nullptr) {
            lineHeight = 12;
        } else if (strcmp(font, kFontTitle) == 0 || (strstr(font, "16x16") != nullptr && strstr(font, "dialog") != nullptr)) {
            lineHeight = 16;
        } else if (strcmp(font, kFontDesc) == 0 || strstr(font, "fnt_d") != nullptr || strstr(font, "fnt_") != nullptr) {
            lineHeight = 14;
        }
    }

    const int charsPerLine = (width > 35) ? (width / 7) : 28;
    const int halfLine = lineHeight / 2;
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

    static uint8_t s_dummyRenderer[0x40] = {0};
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

        int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
        (void)guikit::getExtent(srcCtl, &srcX, &srcY, &srcW, &srcH);
        int w = srcW;
        if (w <= 0) {
            int clientW = 0;
            uint8_t pad = 0;
            (void)guikit::detail::rdInt(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_ClientW, &clientW);
            (void)guikit::detail::rd<uint8_t>(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_Padding, &pad);
            w = clientW - 2 * static_cast<int>(pad);
            if (w <= 0) w = 240;
        }

        PoolEntry* pool = findOrAllocPool(reinterpret_cast<uintptr_t>(listbox));
        if (!pool) return false;

        s_lastPopulateInfo = LastPopulateInfo{};
        void* outControls[kMaxPooledRows] = {nullptr};
        int outCount = 0;

        auto setupTextRow = [&](const char* text, const char* font, const float* col, int extraPad) {
            if (outCount >= static_cast<int>(kMaxPooledRows)) return;
            const size_t rowIdx = static_cast<size_t>(outCount);
            uintptr_t lbl = ensurePooledLabel(*pool, rowIdx);
            if (!lbl) return;

            (void)guikit::setFill(lbl, nullptr);
            (void)guikit::detail::wr<uint32_t>(lbl + guikit::Label_Border, guikit::Border_FillStyle, 0);

            (void)guikit::setRect(lbl, srcX, srcY, w, 32);
            (void)guikit::setFont(lbl, font);
            (void)guikit::setAlign(lbl, kAlignTopLeft);
            (void)guikit::setTextColor(lbl, col[0], col[1], col[2]);
            (void)guikit::setText(lbl, text);

            const int fh = measureTextHeight(lbl, text, w, font);
            const int finalH = fh + extraPad;
            (void)guikit::setRect(lbl, srcX, srcY, w, finalH);
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

        auto setupDividerRow = [&]() {
            if (outCount >= static_cast<int>(kMaxPooledRows)) return;
            const size_t rowIdx = static_cast<size_t>(outCount);
            uintptr_t lbl = ensurePooledLabel(*pool, rowIdx);
            if (!lbl) return;

            (void)guikit::setText(lbl, "");
            (void)guikit::setAlign(lbl, kAlignTopLeft);

            void* img = guikit::imageFor(kDividerFillResRef);
            (void)guikit::setFill(lbl, img, kTealFrame);
            (void)guikit::setAlpha(lbl, 1.0f);
            (void)guikit::detail::wr<uint32_t>(lbl + guikit::Label_Border, guikit::Border_FillStyle, 2);

            (void)guikit::setRect(lbl, srcX, srcY, w, kDividerHeight);
            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_X, srcX);
            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_Y, srcY);
            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_W, w);
            (void)guikit::detail::wr<int>(lbl, guikit::Ctl_H, kDividerHeight);

            (void)guikit::setVisible(lbl, true);
            (void)guikit::setClickable(lbl, false);

            s_lastPopulateInfo.rowHeights[rowIdx] = kDividerHeight;
            s_lastPopulateInfo.rowAligns[rowIdx] = kAlignTopLeft;
            s_lastPopulateInfo.rowFonts[rowIdx][0] = '\0';
            s_lastPopulateInfo.rowTexts[rowIdx][0] = '\0';
            s_lastPopulateInfo.rowIsDivider[rowIdx] = true;
            s_lastPopulateInfo.rowColors[rowIdx][0] = kTealFrame[0];
            s_lastPopulateInfo.rowColors[rowIdx][1] = kTealFrame[1];
            s_lastPopulateInfo.rowColors[rowIdx][2] = kTealFrame[2];

            outControls[outCount++] = reinterpret_cast<void*>(lbl);
        };

        // Row 0: Title
        if (sec.title[0] != '\0') {
            setupTextRow(sec.title, kFontTitle, kCream, 0);
        }

        // Row 1: Subtitle
        if (sec.subtitle[0] != '\0' && strcmp(sec.subtitle, sec.title) != 0) {
            setupTextRow(sec.subtitle, kFontSub, kTealDim, 0);
        }

        // Divider rows removed per directive 2
        (void)setupDividerRow;

        // Row 2: Rules (uses getRulesFont() per directive 3)
        if (sec.rules[0] != '\0') {
            setupTextRow(sec.rules, getRulesFont(), kTealText, 0);
        }

        // Row 3: Body
        if (sec.body[0] != '\0') {
            setupTextRow(sec.body, getBodyFont(), kTealText, 0);
        }

        if (outCount == 0) return false;

        // Height clamping against physical listbox extent (directive 4)
        int lbX = 0, lbY = 0, lbW = 0, lbH = 0;
        (void)guikit::getExtent(reinterpret_cast<uintptr_t>(listbox), &lbX, &lbY, &lbW, &lbH);

        int sumHeights = 0;
        for (int i = 0; i < outCount; ++i) {
            sumHeights += s_lastPopulateInfo.rowHeights[i];
        }

        if (lbH > 0) {
            const int maxVisible = lbH - 10;
            if (sumHeights <= lbH && sumHeights > maxVisible && outCount > 0) {
                int excess = sumHeights - maxVisible;
                for (int i = outCount - 1; i >= 0 && excess > 0; --i) {
                    uintptr_t lbl = reinterpret_cast<uintptr_t>(outControls[i]);
                    int curH = s_lastPopulateInfo.rowHeights[i];
                    int reduce = (curH - 10 > excess) ? excess : (curH > 10 ? curH - 10 : 0);
                    if (reduce > 0) {
                        curH -= reduce;
                        excess -= reduce;
                        s_lastPopulateInfo.rowHeights[i] = curH;
                        (void)guikit::detail::wr<int>(lbl, guikit::Ctl_H, curH);
                        int rX = 0, rY = 0, rW = 0, rH = 0;
                        (void)guikit::getExtent(lbl, &rX, &rY, &rW, &rH);
                        (void)guikit::setRect(lbl, rX, rY, rW, curH);
                    }
                }
            }
        }

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
