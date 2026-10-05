// EmbedTex.h -- embedded-TGA textures registered with the engine as CAurTextureBasic objects (Steam Aspyr swkotor2.exe).
// Header-only, no STL, logger-neutral (set embtex::g_log; null = silent).
//
// Origin: ActionHotbar.cpp fillTexture / ensureTextures / healTextures plus the pure TGA helpers tgaInspect / tgaBgraToRgba of
// HotbarLogic.h, moved here unchanged in behaviour. The action hotbar is a migration candidate to this header (it still carries its own copy).
// DB lesson 290: a GL reset (ResetAll / SetMem) Resets every registered texture (+0xE4 = 0, pixels and GL names freed) and re-queues them;
// if the engine rebuilds first its resource lookup misses our name (+0xE5 = 1), so heal() must Reset and refill; both reset orders are covered.
//
// How it works: a texture is a CAurTextureBasic registered under our name (like the engine's own "NormCubeMap") with decoded RGBA pixels
// already in place and +0xE4 "constructed" set, so the engine never asks the resource manager for it; the texture tick uploads it and frees
// the buffer (engine operator new[]; FreeImage delete[]s +0x44 when +0xE7 == 0). ensure() creates + fills, heal() (every UI tick) refills
// after an engine reset, with a 20-refill give-up per texture. The pooled GUI image handle the hotbar takes after the fill
// (imageFor(resref), which keeps the texture alive) is the CALLER's job; EmbedTex does not depend on GuiKit.
//
// Steam-only (same gate as GuiTip.h): every engine-touching path returns false / no-ops unless k2addr::get() == &kSteam. The object layout
// below is decoded for the Steam exe; engine function addresses are gameaddr:: names only. Host tests (-DEMBTEX_HOST) inject fakes into g_et;
// the Steam gate is steered with k2addr::setForTest (see _shared_src/embtex_test.cpp).
#pragma once
#include "K2Addr.h"
#include <stdint.h>
#include <string.h>

namespace embtex {

#ifdef EMBTEX_HOST
#define EMBTEX_THISCALL
#define EMBTEX_FASTCALL
#else
#define EMBTEX_THISCALL __thiscall
#define EMBTEX_FASTCALL __fastcall
#endif

// ---- CAurTextureBasic fields (Steam decomp; copied from the hotbar) ----
constexpr int ObjSize = 0xF4;        // sizeof(CAurTextureBasic)
constexpr int Tex_Pixels = 0x44;     // unsigned char* RGBA pixels (engine new[])
constexpr int Tex_Uploaded = 0x58;   // GL name; nonzero once the texture tick uploaded it
constexpr int Tex_MipBytes = 0x5C;
constexpr int Tex_W = 0x68, Tex_H = 0x6C, Tex_Bpp = 0x74, Tex_Format = 0x78;
constexpr int Tex_Cube = 0xDF, Tex_Constructed = 0xE4, Tex_Missing = 0xE5, Tex_FromRes = 0xE7, Tex_Tpc = 0xE9;
constexpr int MaxHeals = 20;         // consecutive failed refills before heal() gives up on a texture
constexpr int MaxTex = 16;           // slots in the remembered set

struct Tex { const char* resref; const unsigned char* data; unsigned size; };   // data = whole embedded TGA file (must outlive the process)

// ---- pure TGA helpers (HotbarLogic.h): uncompressed 32-bit -> RGBA ----
// TGA_TOO_LARGE: a dimension above MaxTgaDim (the old 32-bit w*h*4 wrapped for big headers, passed the bounds check and read past the array).
enum TgaStatus { TGA_OK = 0, TGA_NOT_UNCOMPRESSED32, TGA_TRUNCATED, TGA_TOO_LARGE };
constexpr int MaxTgaDim = 4096;   // <= 4096x4096x4 = 64 MiB
struct TgaInfo { int w = 0, h = 0; unsigned off = 0, bytes = 0; };   // pixel data offset, w*h*4 (valid for TGA_OK)
inline TgaStatus tgaInspect(const unsigned char* d, size_t size, TgaInfo* t) {
    if (size < 18 || d[1] != 0 || d[2] != 2 || d[16] != 32) return TGA_NOT_UNCOMPRESSED32;
    int w = d[12] | d[13] << 8, h = d[14] | d[15] << 8; unsigned off = 18u + d[0];
    t->w = w; t->h = h; t->off = off; t->bytes = 0;
    if (w <= 0 || h <= 0) return TGA_TRUNCATED;
    if (w > MaxTgaDim || h > MaxTgaDim) return TGA_TOO_LARGE;
    uint64_t n = (uint64_t)w * (uint64_t)h * 4u;   // 64-bit: <= 2^26 after the limit, and off + n cannot wrap
    if ((uint64_t)off + n > (uint64_t)size) return TGA_TRUNCATED;
    t->bytes = (unsigned)n;
    return TGA_OK;
}
// BGRA -> RGBA, rows as stored (= CResTGA); dst holds t.bytes bytes.
inline void tgaBgraToRgba(const unsigned char* d, const TgaInfo& t, unsigned char* dst) {
    for (unsigned j = 0; j < t.bytes; j += 4) { dst[j] = d[t.off + j + 2]; dst[j + 1] = d[t.off + j + 1]; dst[j + 2] = d[t.off + j]; dst[j + 3] = d[t.off + j + 3]; }
}

// ---- engine table (injectable; defaults = the Steam engine functions by gameaddr:: name) ----
struct Engine {
    void* (*newFn)(unsigned);                                              // engine operator new (cdecl): the CAurTextureBasic object
    void* (*newArrFn)(unsigned);                                           // engine operator new[] (cdecl): the pixel buffer
    void* (EMBTEX_THISCALL* ctorFn)(void*, const char*, const char*);      // CAurTextureBasic ctor(this, name, name), RET 8; registers the name
    void  (EMBTEX_THISCALL* listAddFn)(void*, void*);                      // pending-build list add(list, basic)
    void  (EMBTEX_FASTCALL* resetFn)(void*);                               // CAurTextureBasic::Reset(basic)
    uintptr_t pendingList;                                                 // the pending-texture list object
};
#ifdef EMBTEX_HOST
inline Engine g_et = {nullptr, nullptr, nullptr, nullptr, nullptr, 0};
#else
inline Engine g_et = {reinterpret_cast<void* (*)(unsigned)>(gameaddr::New), reinterpret_cast<void* (*)(unsigned)>(gameaddr::EngineNewArr),
                      reinterpret_cast<void* (__thiscall*)(void*, const char*, const char*)>(gameaddr::TexBasicCtor),
                      reinterpret_cast<void(__thiscall*)(void*, void*)>(gameaddr::TexListAdd),
                      reinterpret_cast<void(__fastcall*)(void*)>(gameaddr::TexReset), gameaddr::TexPendingList};
#endif

inline void (*g_log)(const char* fmt, ...) = nullptr;   // logger-neutral sink; null = silent
#define EMBTEX_LOG(...) do { if (::embtex::g_log) ::embtex::g_log(__VA_ARGS__); } while (0)

inline bool supported() { return k2addr::get() == &kSteam; }   // Steam exe table only (the offsets above are decoded for it)

struct Slot { const Tex* t = nullptr; unsigned char* basic = nullptr; int w = 0, h = 0, heals = 0; };
inline Slot* slots() { static Slot s[MaxTex]; return s; }
inline int& slotCount() { static int n = 0; return n; }

namespace detail {
template <typename T> inline T& at(unsigned char* b, int off) { return *reinterpret_cast<T*>(b + off); }
inline int pendingCount() { return g_et.pendingList ? *reinterpret_cast<int*>(g_et.pendingList + 4) : 0; }
inline bool engineReady() { return g_et.newFn && g_et.newArrFn && g_et.ctorFn && g_et.listAddFn && g_et.resetFn; }

// Decode the TGA into an engine-allocated RGBA buffer, store it in the object's fields, queue it for upload.
inline bool fill(Slot& s, const char* why) {
    const Tex& src = *s.t; TgaInfo ti; TgaStatus ts = tgaInspect(src.data, src.size, &ti);   // 64-bit size math + MaxTgaDim limit
    if (ts == TGA_NOT_UNCOMPRESSED32) { EMBTEX_LOG("tex %s: embedded TGA not uncompressed 32-bit, skipped", src.resref); return false; }
    if (ts == TGA_TRUNCATED) { EMBTEX_LOG("tex %s: embedded TGA truncated (%dx%d, %u bytes)", src.resref, ti.w, ti.h, src.size); return false; }
    if (ts == TGA_TOO_LARGE) { EMBTEX_LOG("tex %s: embedded TGA too large (%dx%d, limit %d), skipped", src.resref, ti.w, ti.h, MaxTgaDim); return false; }
    unsigned char* px = static_cast<unsigned char*>(g_et.newArrFn(ti.bytes));
    if (!px) { EMBTEX_LOG("tex %s: engine malloc(%u) failed", src.resref, ti.bytes); return false; }
    tgaBgraToRgba(src.data, ti, px);   // BGRA -> RGBA, rows as stored (= CResTGA)
    unsigned char* b = s.basic;
    at<unsigned char*>(b, Tex_Pixels) = px;
    at<int>(b, Tex_W) = ti.w; at<int>(b, Tex_H) = ti.h; at<int>(b, Tex_Bpp) = 4; at<int>(b, Tex_Format) = 0; at<int>(b, Tex_MipBytes) = 0;
    b[Tex_Cube] = 0; b[Tex_FromRes] = 0; b[Tex_Tpc] = 0; b[Tex_Missing] = 0; b[Tex_Constructed] = 1;
    g_et.listAddFn(reinterpret_cast<void*>(g_et.pendingList), b);
    s.w = ti.w; s.h = ti.h;
    EMBTEX_LOG("tex %s: %s, %dx%d RGBA queued for upload (basic %p, pending %d)", src.resref, why, ti.w, ti.h, static_cast<void*>(b), pendingCount());
    return true;
}
}  // namespace detail

// Create + fill each texture not yet in the set and remember it for heal(). Returns true only when every one of the n textures was created
// and filled (a failed fill stays registered; heal() keeps retrying up to MaxHeals). False (no engine call) on a non-Steam exe or unready table.
inline bool ensure(const Tex* t, int n) {
    if (!t || n <= 0 || !supported() || !detail::engineReady()) return false;
    bool all = true;
    for (int i = 0; i < n; ++i) {
        int k = -1;
        for (int j = 0; j < slotCount(); ++j) if (!strcmp(slots()[j].t->resref, t[i].resref)) { k = j; break; }
        if (k >= 0) { if (!slots()[k].basic) all = false; continue; }   // already created (idempotent)
        if (slotCount() >= MaxTex) { EMBTEX_LOG("tex %s: slot table full (%d)", t[i].resref, MaxTex); all = false; continue; }
        Slot& s = slots()[slotCount()++]; s = Slot(); s.t = &t[i];
        unsigned char* b = static_cast<unsigned char*>(g_et.newFn(ObjSize));
        if (!b) { EMBTEX_LOG("tex %s: allocation failed", t[i].resref); all = false; continue; }
        memset(b, 0, ObjSize);
        g_et.ctorFn(b, t[i].resref, t[i].resref);
        s.basic = b;
        if (!detail::fill(s, "created")) all = false;
    }
    return all;
}

// Call every UI tick. reset -> refill; engine rebuilt without our pixels (Missing / size changed) -> Reset + refill. DB lesson 290.
// If we see a GL reset first, refill (the duplicate list entry is a no-op: +0xE4 = 1, +0x44 = NULL after the first build).
inline void heal() {
    if (!supported() || !detail::engineReady()) return;
    for (int i = 0; i < slotCount(); ++i) {
        Slot& s = slots()[i]; unsigned char* b = s.basic;
        if (!b) continue;
        bool built = b[Tex_Constructed] != 0; void* px = detail::at<void*>(b, Tex_Pixels); int w = detail::at<int>(b, Tex_W);
        if (built && !px && !b[Tex_Missing] && detail::at<int>(b, Tex_Uploaded)) { s.heals = 0; continue; }   // uploaded: healthy
        if (s.heals >= MaxHeals) continue;   // given up (art missing, labels still work)
        if (!built && !px) { ++s.heals; detail::fill(s, "refilled after a texture reset"); }
        else if (built && !px && (b[Tex_Missing] || w != s.w)) {
            ++s.heals; EMBTEX_LOG("tex %s: rebuilt without our pixels (missing=%d w=%d), resetting", s.t->resref, (int)b[Tex_Missing], w);
            g_et.resetFn(b);
            detail::fill(s, "refilled after a missing rebuild");
        }
        if (s.heals == MaxHeals) EMBTEX_LOG("tex %s: giving up after %d failed refills (built=%d missing=%d pixels=%p); art missing, labels still work",
                                            s.t->resref, MaxHeals, (int)b[Tex_Constructed], (int)b[Tex_Missing], detail::at<void*>(b, Tex_Pixels));   // crossed once: the >= check above skips afterwards
    }
}

// Test seam: forget the remembered set (does not free the objects: the engine owns them for the process lifetime).
inline void clearForTest() { slotCount() = 0; }

}  // namespace embtex
