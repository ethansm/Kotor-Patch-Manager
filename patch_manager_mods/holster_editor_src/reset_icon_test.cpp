// Host test of ResetIcon.h (HW-UX3 L0b): the embedded reset-arrow table (count, names, TGA headers and sizes, pixel sanity), registration
// through EmbedTex with a fake engine, and the pooled image getter. Build/run: python3 scripts/run_tests.py reset_icon
// Expected values are independent literals (32x32, 4140 bytes = 18 header + 4096 pixels + 26 footer).
#ifndef EMBTEX_HOST
#define EMBTEX_HOST 1
#endif
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "ResetIcon.h"
#include "check.h"
#include <stdlib.h>

using namespace hse;
static int nNew = 0, nCtor = 0, nAdd = 0; static char g_ctorName[2][32]; static int g_nLoad = 0; static void* g_loadResult = nullptr;
static void* g_alloc[16]; static int g_nAlloc = 0;   // every fake-engine allocation, freed at the end (LeakSanitizer; the real engine owns these)
static void* track(void* p) { if (g_nAlloc < 16) g_alloc[g_nAlloc++] = p; return p; }
static void* fNew(unsigned n) { ++nNew; return track(calloc(1, n)); }
static void* fNewArr(unsigned n) { return track(malloc(n)); }
static void* fCtor(void* t, const char* a, const char*) { if (nCtor < 2) strncpy(g_ctorName[nCtor], a, 31); ++nCtor; return t; }
static void fAdd(void*, void*) { ++nAdd; }
static void fReset(void*) {}
static void* fLoad(const char*) { ++g_nLoad; return g_loadResult; }

static bool g_btnRet = false, g_lblRet = false; static int g_btnCalls = 0, g_lblCalls = 0;
static bool fBtn() { ++g_btnCalls; return g_btnRet; }
static bool fLbl() { ++g_lblCalls; return g_lblRet; }

int main() {
    // ---- table: two textures, fixed names and order ----
    KT_CHECK_EQ(resetIconTextureCount(), 2);
    const embtex::Tex* t = resetIconTextures();
    KT_CHECK(!strcmp(t[0].resref, "hse_reset")); KT_CHECK(!strcmp(t[1].resref, "hse_reset_hi"));
    KT_CHECK(!strcmp(resetIconResref(false), "hse_reset")); KT_CHECK(!strcmp(resetIconResref(true), "hse_reset_hi"));
    KT_CHECK_EQ(kResetIconSize, 32);
    for (int i = 0; i < 2; ++i) {
        KT_CHECK_EQ(t[i].size, 4140u);
        embtex::TgaInfo ti;
        KT_CHECK_EQ((int)embtex::tgaInspect(t[i].data, t[i].size, &ti), (int)embtex::TGA_OK);
        KT_CHECK_EQ(ti.w, 32); KT_CHECK_EQ(ti.h, 32); KT_CHECK_EQ(ti.off, 18u); KT_CHECK_EQ(ti.bytes, 4096u);
        KT_CHECK_EQ(t[i].data[17], 0x08);                                        // 8 alpha bits, bottom-left origin
        KT_CHECK(!memcmp(t[i].data + t[i].size - 18, "TRUEVISION-XFILE.", 17));  // TGA 2.0 footer
        unsigned opaque = 0, clear = 0; const unsigned char* px = t[i].data + ti.off;
        for (int p = 0; p < 32 * 32; ++p) { unsigned a = px[p * 4 + 3]; if (a == 255) ++opaque; else if (a < 16) ++clear; }
        KT_CHECK(opaque > 60 && opaque < 500);   // a thin line glyph, not a filled square
        KT_CHECK(clear > 300);                   // (near-)transparent background
        KT_CHECK_EQ((int)px[3], 0);              // bottom-left corner is transparent
        KT_CHECK_EQ((int)px[(32 * 32 - 1) * 4 + 3], 0);   // top-right corner is transparent
    }
    KT_CHECK(memcmp(t[0].data, t[1].data, t[0].size) != 0);   // hover variant is a different image

    // ---- registration with a fake engine (Steam table is the default) ----
    static unsigned char pending[16] = {0};
    embtex::g_et.newFn = fNew; embtex::g_et.newArrFn = fNewArr; embtex::g_et.ctorFn = fCtor; embtex::g_et.listAddFn = fAdd; embtex::g_et.resetFn = fReset;
    embtex::g_et.pendingList = reinterpret_cast<uintptr_t>(pending);
    KT_CHECK(resetIconEnsure());
    KT_CHECK_EQ(nNew, 2); KT_CHECK_EQ(nCtor, 2); KT_CHECK_EQ(nAdd, 2); KT_CHECK_EQ(embtex::slotCount(), 2);
    KT_CHECK(!strcmp(g_ctorName[0], "hse_reset") && !strcmp(g_ctorName[1], "hse_reset_hi"));
    KT_CHECK(resetIconEnsure()); KT_CHECK_EQ(nNew, 2);   // idempotent

    // ---- image getter: nullptr until the engine can resolve the name (not cached), then pooled per variant ----
    guikit::g_gk.loadImage = fLoad; g_loadResult = nullptr;
    KT_CHECK(resetIconImage(false) == nullptr && resetIconImage(true) == nullptr);
    int before = g_nLoad;
    KT_CHECK(resetIconImage(false) == nullptr); KT_CHECK_EQ(g_nLoad, before + 1);   // a miss retries
    g_loadResult = reinterpret_cast<void*>(0x4000);
    KT_CHECK(resetIconImage(false) == g_loadResult); KT_CHECK(resetIconImage(true) == g_loadResult);
    int after = g_nLoad; g_loadResult = reinterpret_cast<void*>(0x5000);
    KT_CHECK(resetIconImage(false) == reinterpret_cast<void*>(0x4000)); KT_CHECK_EQ(g_nLoad, after);   // cached
    // ---- resetIconApply: button path first, label path second, false when both fail ----
    g_btnRet = true; g_lblRet = true; g_btnCalls = g_lblCalls = 0;
    KT_CHECK(resetIconApply(fBtn, fLbl)); KT_CHECK_EQ(g_btnCalls, 1); KT_CHECK_EQ(g_lblCalls, 0);   // button took it: label untouched
    g_btnRet = false; g_btnCalls = g_lblCalls = 0;
    KT_CHECK(resetIconApply(fBtn, fLbl)); KT_CHECK_EQ(g_btnCalls, 1); KT_CHECK_EQ(g_lblCalls, 1);   // label clone fallback
    g_lblRet = false; g_btnCalls = g_lblCalls = 0;
    KT_CHECK(!resetIconApply(fBtn, fLbl)); KT_CHECK_EQ(g_btnCalls, 1); KT_CHECK_EQ(g_lblCalls, 1);  // both failed
    KT_CHECK(!resetIconApply(nullptr, nullptr));
    g_lblRet = true; g_lblCalls = 0;
    KT_CHECK(resetIconApply(nullptr, fLbl)); KT_CHECK_EQ(g_lblCalls, 1);
    g_btnRet = true; KT_CHECK(resetIconApply(fBtn, nullptr));
    g_btnRet = false; KT_CHECK(!resetIconApply(fBtn, nullptr));
    for (int i = 0; i < g_nAlloc; ++i) free(g_alloc[i]);
    return kt::Report("reset_icon");
}
