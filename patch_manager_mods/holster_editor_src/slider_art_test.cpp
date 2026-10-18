// Host test of SliderArt.h: the embedded slider table (count, names, resref lengths, TGA headers and sizes, pixel sanity: frame hollow, wedge
// taper, marker centred), registration through EmbedTex with a fake engine, the pooled image getter, the label fill helper and layout helpers.
// Build/run: python3 scripts/run_tests.py slider_art
// Expected values are independent literals (128x16 = 8236 bytes, 8x32 = 1068 bytes: 18 header + w*h*4 pixels + 26 footer).
#ifndef EMBTEX_HOST
#define EMBTEX_HOST 1
#endif
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SliderArt.h"
#include "check.h"
#include <stdlib.h>

using namespace hse;
static int nNew = 0, nCtor = 0, nAdd = 0; static char g_ctorName[3][32]; static int g_nLoad = 0; static void* g_loadResult = nullptr;
static void* g_alloc[16]; static int g_nAlloc = 0;   // every fake-engine allocation, freed at the end (LeakSanitizer; the real engine owns these)
static void* track(void* p) { if (g_nAlloc < 16) g_alloc[g_nAlloc++] = p; return p; }
static void* fNew(unsigned n) { ++nNew; return track(calloc(1, n)); }
static void* fNewArr(unsigned n) { return track(malloc(n)); }
static void* fCtor(void* t, const char* a, const char*) { if (nCtor < 3) strncpy(g_ctorName[nCtor], a, 31); ++nCtor; return t; }
static void fAdd(void*, void*) { ++nAdd; }
static void fReset(void*) {}
static void* fLoad(const char*) { ++g_nLoad; return g_loadResult; }

static unsigned alphaAt(const embtex::Tex& t, int w, int x, int y) {   // y = 0 is the top row; TGA rows are stored bottom-up
    embtex::TgaInfo ti; embtex::tgaInspect(t.data, t.size, &ti);
    return t.data[ti.off + (((size_t)(ti.h - 1 - y)) * w + x) * 4 + 3];
}

int main() {
    // ---- table: three textures, fixed names and order, resrefs fit the 16-char limit ----
    KT_CHECK_EQ(sliderArtTextureCount(), 3);
    const embtex::Tex* t = sliderArtTextures();
    KT_CHECK(!strcmp(t[0].resref, "hse_sld_frame")); KT_CHECK(!strcmp(t[1].resref, "hse_sld_wedge")); KT_CHECK(!strcmp(t[2].resref, "hse_sld_mark"));
    KT_CHECK(!strcmp(sliderArtResref(SA_Frame), "hse_sld_frame")); KT_CHECK(!strcmp(sliderArtResref(SA_Wedge), "hse_sld_wedge"));
    KT_CHECK(!strcmp(sliderArtResref(SA_Mark), "hse_sld_mark")); KT_CHECK(!strcmp(sliderArtResref(3), "")); KT_CHECK(!strcmp(sliderArtResref(-1), ""));
    for (int i = 0; i < 3; ++i) KT_CHECK(strlen(t[i].resref) > 0 && strlen(t[i].resref) <= 16);
    KT_CHECK_EQ(kSliderWideW, 128); KT_CHECK_EQ(kSliderWideH, 16); KT_CHECK_EQ(kSliderMarkW, 8); KT_CHECK_EQ(kSliderMarkH, 32);
    const int W[3] = { 128, 128, 8 }, H[3] = { 16, 16, 32 }; const unsigned SZ[3] = { 8236u, 8236u, 1068u };
    for (int i = 0; i < 3; ++i) {
        KT_CHECK_EQ(t[i].size, SZ[i]);
        embtex::TgaInfo ti;
        KT_CHECK_EQ((int)embtex::tgaInspect(t[i].data, t[i].size, &ti), (int)embtex::TGA_OK);
        KT_CHECK_EQ(ti.w, W[i]); KT_CHECK_EQ(ti.h, H[i]); KT_CHECK_EQ(ti.off, 18u); KT_CHECK_EQ(ti.bytes, (unsigned)(W[i] * H[i] * 4));
        KT_CHECK_EQ(t[i].data[17], 0x08);                                        // 8 alpha bits, bottom-left origin
        KT_CHECK(!memcmp(t[i].data + t[i].size - 18, "TRUEVISION-XFILE.", 17));  // TGA 2.0 footer
        unsigned opaque = 0, clear = 0;
        for (int y = 0; y < H[i]; ++y) for (int x = 0; x < W[i]; ++x) { unsigned a = alphaAt(t[i], W[i], x, y); if (a == 255) ++opaque; else if (a < 16) ++clear; }
        KT_CHECK(opaque > 0);                              // non-empty alpha
        KT_CHECK(clear > (unsigned)(W[i] * H[i]) / 4);     // (near-)transparent background: none of these is a filled rectangle
    }

    // ---- frame: hollow outline, all four edges drawn, corners rounded ----
    KT_CHECK_EQ(alphaAt(t[0], 128, 64, 0), 255u); KT_CHECK_EQ(alphaAt(t[0], 128, 64, 15), 255u);   // top and bottom edges
    KT_CHECK_EQ(alphaAt(t[0], 128, 0, 8), 255u); KT_CHECK_EQ(alphaAt(t[0], 128, 127, 8), 255u);    // left and right edges
    KT_CHECK_EQ(alphaAt(t[0], 128, 64, 8), 0u); KT_CHECK_EQ(alphaAt(t[0], 128, 20, 5), 0u);        // transparent inside
    KT_CHECK_EQ(alphaAt(t[0], 128, 0, 0), 0u); KT_CHECK_EQ(alphaAt(t[0], 128, 127, 15), 0u);       // rounded corners
    { const unsigned char* d = t[0].data + 18 + (size_t)(15 - 0) * 128 * 4 + 64 * 4;               // top edge pixel (row 0 is stored last): teal, G > R
      KT_CHECK(d[1] > d[2] && d[1] > d[0]); }

    // ---- wedge: thin at the left, tall at the right; per-column coverage never shrinks; nothing outside the frame's inner edge ----
    unsigned prev = 0, first = 0, last = 0;
    for (int x = 0; x < 128; ++x) {
        unsigned sum = 0; for (int y = 0; y < 16; ++y) sum += alphaAt(t[1], 128, x, y);
        if (x < 4 || x > 122) { KT_CHECK_EQ(sum, 0u); continue; }   // 5 px margin each side (columns 5..122 carry the wedge)
        KT_CHECK(sum >= prev); prev = sum;
        if (x == 8) first = sum;
        if (x == 120) last = sum;
    }
    KT_CHECK(first > 0 && last > 3 * first);                                  // clearly tapered
    KT_CHECK(alphaAt(t[1], 128, 120, 4) == 255u && alphaAt(t[1], 128, 8, 4) == 0u);   // tall end covers row 4, thin end does not
    for (int x = 0; x < 128; ++x) KT_CHECK_EQ(alphaAt(t[1], 128, x, 0), 0u);          // top and bottom rows are the frame's
    for (int x = 0; x < 128; ++x) KT_CHECK_EQ(alphaAt(t[1], 128, x, 15), 0u);

    // ---- marker: white, vertically and horizontally symmetric (centred), arrowheads at both ends ----
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 4; ++x) KT_CHECK_EQ(alphaAt(t[2], 8, x, y), alphaAt(t[2], 8, 7 - x, y));   // left/right mirror: centred on x = 4
        for (int x = 0; x < 8; ++x) KT_CHECK_EQ(alphaAt(t[2], 8, x, y), alphaAt(t[2], 8, x, 31 - y));  // top/bottom mirror: double arrow
    }
    KT_CHECK_EQ(alphaAt(t[2], 8, 3, 16), 255u); KT_CHECK_EQ(alphaAt(t[2], 8, 4, 16), 255u);   // 2 px bar in the middle
    KT_CHECK_EQ(alphaAt(t[2], 8, 1, 16), 0u); KT_CHECK_EQ(alphaAt(t[2], 8, 6, 16), 0u);
    KT_CHECK(alphaAt(t[2], 8, 0, 5) > 0 && alphaAt(t[2], 8, 7, 26) > 0);                      // arrowhead bases span the full width
    KT_CHECK_EQ(alphaAt(t[2], 8, 0, 0), 0u);                                                  // apex is narrow
    { embtex::TgaInfo ti; embtex::tgaInspect(t[2].data, t[2].size, &ti);
      for (int p = 0; p < 8 * 32; ++p) { const unsigned char* d = t[2].data + ti.off + p * 4; if (d[3]) KT_CHECK(d[0] == 255 && d[1] == 255 && d[2] == 255); } }
    for (int i = 0; i < 2; ++i) KT_CHECK(memcmp(t[i].data, t[i + 1].data, t[i].size < t[i + 1].size ? t[i].size : t[i + 1].size) != 0);   // distinct images

    // ---- registration with a fake engine (Steam table is the default) ----
    static unsigned char pending[16] = {0};
    embtex::g_et.newFn = fNew; embtex::g_et.newArrFn = fNewArr; embtex::g_et.ctorFn = fCtor; embtex::g_et.listAddFn = fAdd; embtex::g_et.resetFn = fReset;
    embtex::g_et.pendingList = reinterpret_cast<uintptr_t>(pending);
    KT_CHECK(sliderArtEnsure());
    KT_CHECK_EQ(nNew, 3); KT_CHECK_EQ(nCtor, 3); KT_CHECK_EQ(nAdd, 3); KT_CHECK_EQ(embtex::slotCount(), 3);
    KT_CHECK(!strcmp(g_ctorName[0], "hse_sld_frame") && !strcmp(g_ctorName[1], "hse_sld_wedge") && !strcmp(g_ctorName[2], "hse_sld_mark"));
    KT_CHECK(sliderArtEnsure()); KT_CHECK_EQ(nNew, 3);   // idempotent

    // ---- image getter: nullptr until the engine can resolve the name (not cached), then pooled per part ----
    guikit::g_gk.loadImage = fLoad; g_loadResult = nullptr;
    KT_CHECK(sliderArtImage(SA_Frame) == nullptr && sliderArtImage(SA_Wedge) == nullptr && sliderArtImage(SA_Mark) == nullptr);
    int before = g_nLoad;
    KT_CHECK(sliderArtImage(SA_Mark) == nullptr); KT_CHECK_EQ(g_nLoad, before + 1);   // a miss retries
    KT_CHECK(sliderArtImage(3) == nullptr && sliderArtImage(-1) == nullptr); KT_CHECK_EQ(g_nLoad, before + 1);   // out of range: no engine call
    // ---- sliderArtFill with no texture yet / no control: false, no write attempt ----
    KT_CHECK(!sliderArtFill(0x1000, SA_Frame)); KT_CHECK(!sliderArtApply(0x1000, 0x2000, 0x3000));
    g_loadResult = reinterpret_cast<void*>(0x4000);
    KT_CHECK(sliderArtImage(SA_Frame) == g_loadResult); KT_CHECK(sliderArtImage(SA_Wedge) == g_loadResult); KT_CHECK(sliderArtImage(SA_Mark) == g_loadResult);
    int after = g_nLoad; g_loadResult = reinterpret_cast<void*>(0x5000);
    KT_CHECK(sliderArtImage(SA_Frame) == reinterpret_cast<void*>(0x4000)); KT_CHECK_EQ(g_nLoad, after);   // cached
    // texture exists, but a null control or a control failing the border-vtable gate (no engine table in the host) is refused
    KT_CHECK(!sliderArtFill(0, SA_Frame)); KT_CHECK(!sliderArtFill(0x1000, SA_Wedge)); KT_CHECK(!sliderArtFill(0x1000, 7));
    KT_CHECK_EQ(sliderArtApply(0, 0, 0), 0); KT_CHECK_EQ(sliderArtApply(0x1000, 0x2000, 0x3000), 0);

    // ---- layout helpers ----
    KT_CHECK(sliderWedgeWidth(100.0f, 0.25f) == 25.0f); KT_CHECK(sliderWedgeWidth(100.0f, -1.0f) == 0.0f); KT_CHECK(sliderWedgeWidth(100.0f, 2.0f) == 100.0f);
    KT_CHECK(sliderWedgeWidth(100.0f, 0.0f / 0.0f) == 0.0f);   // NaN
    KT_CHECK(sliderMarkLeft(10.0f, 100.0f, 0.5f) == 10.0f + 50.0f - 4.0f);   // centred on the 50% point, default 8 px marker
    KT_CHECK(sliderMarkLeft(10.0f, 100.0f, 0.0f, 6.0f) == 7.0f); KT_CHECK(sliderMarkLeft(10.0f, 100.0f, 1.0f, 6.0f) == 107.0f);
    KT_CHECK(sliderMarkLeft(10.0f, 100.0f, 5.0f, 6.0f) == 107.0f);   // clamped
    for (int i = 0; i < g_nAlloc; ++i) free(g_alloc[i]);
    return kt::Report("slider_art");
}
