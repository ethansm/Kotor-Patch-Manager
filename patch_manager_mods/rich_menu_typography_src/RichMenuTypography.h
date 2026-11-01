#pragma once

#include <cstdint>
#include <kpatch_api.h>

#ifndef __cdecl
#if defined(_MSC_VER) || defined(__MINGW32__)
#define __cdecl __cdecl
#elif defined(__i386__)
#define __cdecl __attribute__((cdecl))
#else
#define __cdecl
#endif
#endif

extern "C" {
void __cdecl RmtOverrideGlyphColor(uintptr_t ebp);
int __cdecl RmtAddControls2(void* listbox, void** pControls, int count,
                           int bAutoDelete, int bRestoreExtents, int bVariableItemHeights,
                           uintptr_t origFnAddr);
void __cdecl KPatchInit(const KPatchApi* api);
}

namespace rmt {
void setInlineColoring(bool enabled) noexcept;
bool isInlineColoring() noexcept;
void setRichDescBox(bool enabled) noexcept;
bool isRichDescBox() noexcept;
void setVerbose(bool verbose) noexcept;
bool isVerbose() noexcept;
void resetOverrideCount() noexcept;
uint32_t getOverrideCount() noexcept;
void resetDescPopulateCount() noexcept;
uint32_t getDescPopulateCount() noexcept;
} // namespace rmt
