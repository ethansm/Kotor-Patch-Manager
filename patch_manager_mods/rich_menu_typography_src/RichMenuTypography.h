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
void __cdecl KPatchInit(const KPatchApi* api);
}

namespace rmt {
void setInlineColoring(bool enabled) noexcept;
bool isInlineColoring() noexcept;
void setVerbose(bool verbose) noexcept;
bool isVerbose() noexcept;
void resetOverrideCount() noexcept;
uint32_t getOverrideCount() noexcept;
} // namespace rmt
