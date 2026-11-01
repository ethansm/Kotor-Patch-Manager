// RichMenuTypography.cpp -- KPM DLL entry and per-glyph inline color & description listbox hooks (Doc 53 RMT1, RMT2).
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

namespace {
bool g_inlineColoring = true;
bool g_richDescBox = true;
bool g_verbose = false;
uint32_t g_overrideCount = 0;
uint32_t g_descPopulateCount = 0;
constexpr uint32_t kMaxStartupOverrides = 40;
} // namespace

namespace rmt {
void setInlineColoring(bool enabled) noexcept { g_inlineColoring = enabled; }
bool isInlineColoring() noexcept { return g_inlineColoring; }
void setRichDescBox(bool enabled) noexcept { g_richDescBox = enabled; }
bool isRichDescBox() noexcept { return g_richDescBox; }
void setVerbose(bool verbose) noexcept { g_verbose = verbose; }
bool isVerbose() noexcept { return g_verbose; }
void resetOverrideCount() noexcept { g_overrideCount = 0; }
uint32_t getOverrideCount() noexcept { return g_overrideCount; }
void resetDescPopulateCount() noexcept { g_descPopulateCount = 0; }
uint32_t getDescPopulateCount() noexcept { return g_descPopulateCount; }
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

    if (newCol != baseCol) {
        if (g_verbose || g_overrideCount < kMaxStartupOverrides) {
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
        if (KpLog_HasCore()) {
            if (g_verbose || g_descPopulateCount <= 20) {
                KP_LOGV(chMain, KPLOG_DEBUG, "RichDescBox populated listbox=%p popCount=%u",
                        listbox, g_descPopulateCount);
            }
        }
        return 1;
    }
    return 0;
}

extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    const char* const iniPath = ".\\rich_menu_typography.ini";
    g_inlineColoring = GetPrivateProfileIntA("General", "InlineColoring", 1, iniPath) != 0;
    g_richDescBox = GetPrivateProfileIntA("General", "RichDescBox", 1, iniPath) != 0;
    g_verbose = GetPrivateProfileIntA("General", "Verbose", 0, iniPath) != 0;

    static char s_bodyFontBuf[32] = {0};
    GetPrivateProfileStringA("General", "BodyFont", rmt::kFontDesc, s_bodyFontBuf, sizeof(s_bodyFontBuf), iniPath);
    rmt::setBodyFont(s_bodyFontBuf);

    if (KpLog_HasCore()) {
        KP_LOG(chMain, "INIT rich-menu-typography inlineColoring=%d richDescBox=%d bodyFont=%s verbose=%d",
               g_inlineColoring ? 1 : 0, g_richDescBox ? 1 : 0, rmt::getBodyFont(), g_verbose ? 1 : 0);
    }
}

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID /*reserved*/) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
    }
    return TRUE;
}
#endif
