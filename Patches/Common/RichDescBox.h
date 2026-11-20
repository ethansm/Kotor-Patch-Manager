#pragma once

#ifndef _WIN32
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#endif

#include "GameAddr.h"
#include "GuiKit.h"
#include "RichTextClassifier.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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
constexpr float kDividerTint[3] = { 0.0510f, 0.3490f, 0.2710f };   // ItemHoverCard kTealFrame
constexpr float kVanillaDividerTint[3] = { 0.4f, 0.302f, 0.0f };   // vanilla store description border (store_p.gui)
// Section dividers (config page "DividerMode"): 0 off, 1 between header group|rules and rules|body, 2 also between title and subtitle.
inline int& dividerMode() noexcept { static int v = 1; return v; }
constexpr const char* kDividerFillResRef = "uibit_fill_16g";
constexpr size_t kMaxPooledRows = 8;   // 4 text rows + up to 3 dividers
constexpr size_t kMaxListBoxPools = 32;

// ListBox & Label offsets
constexpr int kCtlOff_ParentPanel = 0x38;
constexpr int kLabelOff_TextParams = 0xF0;
constexpr int kLabelOff_Renderer = 0xEC;
constexpr int kTextVtOff_GetFormattedHeight = 0x08; // slot 2 of CSWGuiText vtable at label + Label_Text
constexpr int kTextObjOff_W = 0x0C;
constexpr int kTextObjOff_H = 0x10;
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
inline void (*g_probeLog)(const char*) = nullptr;
// Optional item-name title hook. When set and it returns true with a non-empty name, the title row shows
// that name instead of the engine header (the old header is demoted to the subtitle when that slot is free).
inline bool (*g_itemTitleFn)(void* listbox, char* out, int cap) = nullptr;

inline void probeLogf(const char* fmt, ...) {
    if (!g_probeLog) return;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = '\0';
    g_probeLog(buf);
}

struct ProbeDedupeState {
    uintptr_t listbox = 0;
    size_t totalTextLen = 0;
    int clientH = 0;
    bool hasPrevious = false;
};
inline ProbeDedupeState s_lastProbeState = {};
// Probe builds set this so every populate logs (pairs log lines with screenshots).
inline bool g_probeEveryPopulate = false;
inline uint32_t s_probeSeq = 0;

// Printable copy of up to n bytes of s for probe lines: control bytes become '.', '|' for newline.
inline void probeEscape(const char* s, size_t n, char* out, size_t outCap) noexcept {
    if (!out || outCap == 0) return;
    size_t o = 0;
    for (size_t i = 0; s && i < n && s[i] && o + 1 < outCap; ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        out[o++] = (c == '\n' || c == '\r') ? '|' : ((c < 0x20 || c > 0x7E) ? '.' : static_cast<char>(c));
    }
    out[o] = '\0';
}

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
    s_lastProbeState = ProbeDedupeState{};
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
                // Ctl rects, listbox client size and GetFormattedHeight are all physical px
                // (probe 2026-11-03: reserved = raw/2 + gap on 178/178 rows caused overlap + cut-off).
                if (physicalH > 0) {
                    (void)scalar;
                    return physicalH;
                }
            }
        }
#endif
    }
    if (!text || !*text) return 0;

    int lineHeight = 14;
    int heurScale = 1;
#ifndef GUIKIT_HOST
    // Heuristic below is in 1x px; engine rects are physical px.
    heurScale = static_cast<int>(getEngineTextScalar() + 0.5f);
    if (heurScale < 1) heurScale = 1;
#endif
    if (font) {
        if (strcmp(font, kFontSub) == 0 || strstr(font, "10x10") != nullptr) {
            lineHeight = 8;
        } else if (strcmp(font, kFontTitle) == 0 || (strstr(font, "16x16") != nullptr && strstr(font, "dialog") != nullptr)) {
            lineHeight = 12;
        } else if (strcmp(font, kFontDesc) == 0 || strstr(font, "fnt_d") != nullptr || strstr(font, "fnt_") != nullptr) {
            lineHeight = 14;
        }
    }

    const int charW = ((lineHeight >= 14) ? 7 : (lineHeight >= 12 ? 6 : 5)) * heurScale;
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
    return totalHeight * heurScale;
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

        if (g_itemTitleFn) {
            char name[sizeof sec.title] = {};
            if (g_itemTitleFn(listbox, name, static_cast<int>(sizeof name))) {
                name[sizeof name - 1] = '\0';
                if (name[0] != '\0') {
                    if (sec.title[0] != '\0' && sec.subtitle[0] == '\0') {
                        memcpy(sec.subtitle, sec.title, sizeof sec.subtitle);
                        sec.subtitle[sizeof sec.subtitle - 1] = '\0';
                    }
                    memcpy(sec.title, name, sizeof sec.title);
                    sec.title[sizeof sec.title - 1] = '\0';
                }
            }
        }

        int clientW = 0;
        uint8_t pad = 0;
        (void)guikit::detail::rdInt(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_ClientW, &clientW);
        (void)guikit::detail::rd<uint8_t>(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_Padding, &pad);
        int lbItemW = clientW - 2 * static_cast<int>(pad);
        int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
        (void)guikit::getExtent(srcCtl, &srcX, &srcY, &srcW, &srcH);
        // Lay out at the listbox item width the engine draws at. Capping to the
        // source label width (srcW) made heights measure at ~1/4 of the drawn wrap.
        int w = lbItemW;
        if (w <= 0) {
            w = (srcW > 0) ? srcW : 240;
        }

        int clientH = 0;
        (void)guikit::detail::rdInt(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_ClientH, &clientH);
        if (clientH <= 0) {
            (void)guikit::getExtent(reinterpret_cast<uintptr_t>(listbox), nullptr, nullptr, nullptr, &clientH);
        }
        if (clientH <= 0) clientH = 140;

        int screenW = 0;
#ifndef GUIKIT_HOST
        (void)saferead::readAt<int>(guikit::detail::SiteBase, gameaddr::ScreenW, 0, &screenW);
#endif

        const float textScalar = getEngineTextScalar();
        (void)textScalar;

        const size_t totalTextLen = strlen(sec.title) + strlen(sec.subtitle) + strlen(sec.rules) + strlen(sec.body);
        const uintptr_t lbPtr = reinterpret_cast<uintptr_t>(listbox);
        const bool logProbe = (g_probeEveryPopulate ||
                               !s_lastProbeState.hasPrevious ||
                               s_lastProbeState.listbox != lbPtr ||
                               s_lastProbeState.totalTextLen != totalTextLen ||
                               s_lastProbeState.clientH != clientH);
        if (logProbe) {
            s_lastProbeState.listbox = lbPtr;
            s_lastProbeState.totalTextLen = totalTextLen;
            s_lastProbeState.clientH = clientH;
            s_lastProbeState.hasPrevious = true;
        }

        uint32_t probeSeq = 0;
        if (logProbe) {
            probeSeq = ++s_probeSeq;
            const size_t rawLen = strlen(rawText);
            int nLt = 0, nCtl = 0;
            for (size_t i = 0; i < rawLen; ++i) {
                const unsigned char c = static_cast<unsigned char>(rawText[i]);
                if (c == '<') ++nLt;
                if ((c < 0x20 && c != '\n' && c != '\r') || c > 0x7E) ++nCtl;
            }
            char head[41] = {0}, tail[41] = {0};
            probeEscape(rawText, 40, head, sizeof(head));
            probeEscape(rawText + (rawLen > 40 ? rawLen - 40 : 0), 40, tail, sizeof(tail));
            probeLogf("RMTPROBE id seq=%u lb=%p src=%p rawLen=%zu secT=%zu secS=%zu secR=%zu secB=%zu lt=%d ctl=%d head=%s tail=%s",
                      probeSeq, listbox, pControls[0], rawLen,
                      strlen(sec.title), strlen(sec.subtitle), strlen(sec.rules), strlen(sec.body),
                      nLt, nCtl, head, tail);
            int shown = 0;
            for (size_t i = 0; i < rawLen && shown < 3; ++i) {
                if (rawText[i] == '<') {
                    char ctx[41] = {0};
                    probeEscape(rawText + i, 32, ctx, sizeof(ctx));
                    probeLogf("RMTPROBE lt seq=%u at=%zu ctx=%s", probeSeq, i, ctx);
                    ++shown;
                }
            }
        }

        PoolEntry* pool = findOrAllocPool(reinterpret_cast<uintptr_t>(listbox));
        if (!pool) return false;

        s_lastPopulateInfo = LastPopulateInfo{};
        void* outControls[kMaxPooledRows] = {nullptr};
        int outCount = 0;

        auto measureRowH = [&](const char* kind, uintptr_t lbl, const char* text, const char* font) -> int {
            (void)guikit::setRect(lbl, srcX, srcY, w, 32);
            (void)guikit::setFont(lbl, font);
            (void)guikit::setAlign(lbl, kAlignTopLeft);
            (void)guikit::setText(lbl, text);

            int engH = 0;
            int txtW = 0;
            int txtH = 0;
            int rawH = 0;
#ifndef GUIKIT_HOST
            if (guikit::detail::textObjOk(lbl)) {
                const uintptr_t textObj = lbl + guikit::Label_Text;
                (void)guikit::detail::rdInt(textObj, kTextObjOff_W, &txtW);
                (void)guikit::detail::rdInt(textObj, kTextObjOff_H, &txtH);
                uint32_t vt = 0;
                if (guikit::detail::rd32(textObj, 0, &vt) && vt) {
                    uint32_t fn = 0;
                    if (guikit::detail::rd32(vt, kTextVtOff_GetFormattedHeight, &fn) && fn) {
                        int physicalH = reinterpret_cast<int(__thiscall*)(void*)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(textObj));
                        rawH = physicalH;
                        if (physicalH > 0) {
                            engH = physicalH;  // physical px, same space as Ctl_H and clientH
                        }
                    }
                }
            }
#endif
            const int fbH = measureTextHeight(lbl, text, w, font);
            const int finalH = (engH > 0) ? engH : fbH;
            const char* const src = (engH > 0) ? "engine" : "fallback";

            if (logProbe) {
                char head[21] = {0};
                const size_t textLen = text ? strlen(text) : 0;
                const size_t copyLen = (textLen < 20) ? textLen : 20;
                for (size_t i = 0; i < copyLen; ++i) {
                    const char c = text[i];
                    head[i] = (c == '\n' || c == '\r') ? '|' : c;
                }
                head[copyLen] = '\0';
                char tailBuf[25] = {0};
                probeEscape(text + (textLen > 24 ? textLen - 24 : 0), 24, tailBuf, sizeof(tailBuf));
                probeLogf("RMTPROBE raw seq=%u kind=%s rawH=%d engH=%d fbH=%d scalar=%d w=%d len=%zu tail=%s",
                          probeSeq, kind ? kind : "unknown", rawH, engH, fbH,
                          static_cast<int>(textScalar * 100.0f), w, textLen, tailBuf);
                probeLogf("RMTPROBE row kind=%s w=%d clientH=%d pad=%u src=%s engH=%d fbH=%d txtW=%d txtH=%d scalar=%d screenW=%d len=%zu head=%s",
                          kind ? kind : "unknown",
                          w,
                          clientH,
                          static_cast<unsigned>(pad),
                          src,
                          engH,
                          fbH,
                          txtW,
                          txtH,
                          static_cast<int>(textScalar * 100.0f),
                          screenW,
                          textLen,
                          head);
            }

            return finalH;
        };

        float origCol[3] = { kTealText[0], kTealText[1], kTealText[2] };
        (void)guikit::getTextColor(srcCtl, origCol);
        const bool isTeal = isDefaultTeal(origCol[0], origCol[1], origCol[2]);

        // Non-teal menus (e.g. the store's gold text) keep the vanilla control colour on every row, so the box matches vanilla.
        const float* titleCol = isTeal ? kCream : origCol;
        const float* rulesCol = isTeal ? kTealText : origCol;
        const float* bodyCol  = isTeal ? kTealText : origCol;

        const bool hasTitle    = (sec.title[0] != '\0');
        const bool hasSubtitle = (sec.subtitle[0] != '\0' && strcmp(sec.subtitle, sec.title) != 0);
        const bool hasRules    = (sec.rules[0] != '\0');
        const bool hasBody     = (sec.body[0] != '\0');

        // Row plan: text rows in order, with a thin divider row where dmode asks for one and both neighbours exist.
        enum : int { kRowTitle, kRowSub, kRowRules, kRowBody, kRowDiv };
        struct PlanRow { int kind; uintptr_t lbl; };
        PlanRow plan[kMaxPooledRows] = {};
        int nPlan = 0;
        int titleH = 0, subH = 0, rulesH = 0, bodyH = 0;
        int activeCount = 0;
        int nDiv = 0;
        const int dmode = dividerMode();
        void* const divImg = (dmode > 0) ? guikit::imageFor(kDividerFillResRef) : nullptr;
        auto addDivider = [&]() {
            if (!divImg || nPlan >= static_cast<int>(kMaxPooledRows)) return;
            const uintptr_t l = ensurePooledLabel(*pool, static_cast<size_t>(nPlan));
            if (!l) return;
            plan[nPlan++] = { kRowDiv, l };
            ++activeCount;
            ++nDiv;
        };
        auto addText = [&](int kind, const char* text, const char* font, const char* what, int* hOut) {
            const uintptr_t l = ensurePooledLabel(*pool, static_cast<size_t>(nPlan));
            if (!l) return;
            *hOut = measureRowH(what, l, text, font);
            plan[nPlan++] = { kind, l };
            ++activeCount;
        };

        if (hasTitle) addText(kRowTitle, sec.title, kFontTitle, "title", &titleH);
        if (hasSubtitle) {
            if (hasTitle && dmode >= 2) addDivider();
            addText(kRowSub, sec.subtitle, kFontSub, "sub", &subH);
        }
        if (hasRules) {
            if ((hasTitle || hasSubtitle) && dmode >= 1) addDivider();
            addText(kRowRules, sec.rules, getRulesFont(), "rules", &rulesH);
        }
        if (hasBody) {
            if (hasRules && dmode >= 1) addDivider();
            else if (!hasRules && (hasTitle || hasSubtitle) && dmode >= 1) addDivider();
            addText(kRowBody, sec.body, getBodyFont(), "body", &bodyH);
        }

        const int naturalSum = titleH + subH + rulesH + bodyH + nDiv * kDividerHeight;

        int titleGap = 0, subGap = 0, rulesGap = 0;
        const int desTitleGap = (hasTitle && (hasSubtitle || hasRules || hasBody)) ? 4 : 0;
        const int desSubGap   = (hasSubtitle && (hasRules || hasBody)) ? 3 : 0;
        const int desRulesGap = (hasRules && hasBody) ? 4 : 0;
        const int numGaps = (desTitleGap > 0 ? 1 : 0) + (desSubGap > 0 ? 1 : 0) + (desRulesGap > 0 ? 1 : 0);

        // Engine places item 0 at +pad and adds pad after every item (no trailing pad counted by
        // the listbox itself, but the leading pad is real), so content = sum(h) + n*pad.
        const int padTotal = activeCount * static_cast<int>(pad);
        const int effectiveLimit = (clientH > padTotal) ? (clientH - padTotal) : clientH;

        if (numGaps > 0) {
            const int totalDesiredGaps = desTitleGap + desSubGap + desRulesGap;
            const int remainingSpace = effectiveLimit - naturalSum;

            if (remainingSpace >= totalDesiredGaps) {
                // Ample headroom: fit comfortably on one page with full gaps
                titleGap = desTitleGap;
                subGap   = desSubGap;
                rulesGap = desRulesGap;
            } else if (remainingSpace >= 0) {
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

        if (logProbe) {
            const int gaps = titleGap + subGap + rulesGap;
            const int total = naturalSum + gaps + padTotal;
            const int overflow = total - clientH;
            probeLogf("RMTPROBE sum seq=%u titleH=%d subH=%d rulesH=%d bodyH=%d naturalSum=%d padTotal=%d clientH=%d titleGap=%d subGap=%d rulesGap=%d total=%d overflow=%d",
                      probeSeq, titleH, subH, rulesH, bodyH, naturalSum, padTotal, clientH, titleGap, subGap, rulesGap, total, overflow);
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

        auto applyDividerRow = [&](uintptr_t lbl) {
            if (!lbl || outCount >= static_cast<int>(kMaxPooledRows)) return;
            const size_t rowIdx = static_cast<size_t>(outCount);
            (void)guikit::setText(lbl, "");
            // Teal menus keep the teal rule. Cyan-family menus (workbench, crafting) get a half-bright cyan rule;
            // every other non-teal menu (store gold) keeps vanilla's gold border tint.
            const float cyanRule[3] = { origCol[0] * 0.5f, origCol[1] * 0.5f, origCol[2] * 0.5f };
            const float* divTint = isTeal ? kDividerTint : (isCyanFamily(origCol[0], origCol[1], origCol[2]) ? cyanRule : kVanillaDividerTint);
            (void)guikit::setFill(lbl, divImg, divTint);
            (void)guikit::setFillStyle(lbl, 2);
            (void)guikit::setBorderTint(lbl, divTint);
            (void)guikit::setAlpha(lbl, 1.0f);
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
            outControls[outCount++] = reinterpret_cast<void*>(lbl);
        };

        for (int i = 0; i < nPlan; ++i) {
            switch (plan[i].kind) {
                case kRowTitle: applyTextRow(plan[i].lbl, sec.title, kFontTitle, titleCol, titleH, titleGap); break;
                case kRowSub:   applyTextRow(plan[i].lbl, sec.subtitle, kFontSub, isTeal ? kTealDim : origCol, subH, subGap); break;
                case kRowRules: applyTextRow(plan[i].lbl, sec.rules, getRulesFont(), rulesCol, rulesH, rulesGap); break;
                case kRowBody:  applyTextRow(plan[i].lbl, sec.body, getBodyFont(), bodyCol, bodyH, 0); break;
                default:        applyDividerRow(plan[i].lbl); break;
            }
        }

        if (outCount == 0) return false;

        s_lastPopulateInfo.listbox = reinterpret_cast<uintptr_t>(listbox);
        s_lastPopulateInfo.rowCount = outCount;

        (void)guikit::detail::wr<uint16_t>(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_TopItem, 0);

        s_inPopulate = true;
        origAddControls2(listbox, outControls, outCount, /*bAutoDelete=*/0, /*bRestoreExtents=*/0, /*bVariableItemHeights=*/1);
        s_inPopulate = false;

        (void)guikit::detail::wr<uint16_t>(reinterpret_cast<uintptr_t>(listbox), kListBoxOff_TopItem, 0);

#ifndef GUIKIT_HOST
        if (logProbe) {
            // Readback after the engine stacked the rows: real Y/H vs what we reserved,
            // and the engine's own formatted height of the text actually set on each label.
            const uintptr_t lb = reinterpret_cast<uintptr_t>(listbox);
            uint32_t arr = 0;
            int n = 0;
            int visible = 0;
            uint16_t topItem = 0xFFFF;
            (void)guikit::detail::rd32(lb, 0x2AC, &arr);
            (void)guikit::detail::rdInt(lb, 0x2B0, &n);
            (void)guikit::detail::rdInt(lb, 0x2D4, &visible);
            (void)guikit::detail::rd<uint16_t>(lb, kListBoxOff_TopItem, &topItem);
            int prevY = 0, prevH = 0, overlaps = 0, lastBottom = 0;
            for (int i = 0; arr && i < n && i < 8; ++i) {
                uint32_t cp = 0;
                if (!guikit::detail::rd32(static_cast<uintptr_t>(arr), static_cast<int>(i * 4), &cp) || !cp) continue;
                int y = -1, h = -1, cw = -1, chk = -1;
                (void)guikit::detail::rdInt(cp, guikit::Ctl_Y, &y);
                (void)guikit::detail::rdInt(cp, guikit::Ctl_H, &h);
                (void)guikit::detail::rdInt(cp, guikit::Ctl_W, &cw);
                int fmtRaw = -1;
                const uintptr_t textObj = static_cast<uintptr_t>(cp) + guikit::Label_Text;
                uint32_t tvt = 0, tfn = 0;
                if (guikit::detail::textObjOk(static_cast<uintptr_t>(cp)) &&
                    guikit::detail::rd32(textObj, 0, &tvt) && tvt &&
                    guikit::detail::rd32(tvt, kTextVtOff_GetFormattedHeight, &tfn) && tfn) {
                    fmtRaw = reinterpret_cast<int(__thiscall*)(void*)>(static_cast<uintptr_t>(tfn))(reinterpret_cast<void*>(textObj));
                }
                const bool ov = (i > 0) && (y < prevY + prevH);
                if (ov) ++overlaps;
                const bool clipRaw = (fmtRaw > h);
                const bool clipDiv = (textScalar > 0.01f) && (fmtRaw > 0) && (static_cast<int>(fmtRaw / textScalar + 0.99f) > h);
                (void)chk;
                probeLogf("RMTPROBE stack seq=%u i=%d y=%d h=%d w=%d fmtRaw=%d overlap=%d clipIfRaw=%d clipIfDiv=%d",
                          probeSeq, i, y, h, cw, fmtRaw, ov ? 1 : 0, clipRaw ? 1 : 0, clipDiv ? 1 : 0);
                prevY = y; prevH = h;
                lastBottom = y + h;
            }
            probeLogf("RMTPROBE stacksum seq=%u rows=%d visible=%d top=%u clientH=%d lastBottom=%d overlaps=%d",
                      probeSeq, n, visible, static_cast<unsigned>(topItem), clientH, lastBottom, overlaps);
        }
#endif

        return true;
    }
};

} // namespace rmt
