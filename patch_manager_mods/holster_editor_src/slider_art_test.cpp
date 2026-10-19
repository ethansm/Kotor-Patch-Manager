// Host test of SliderArt.h: the embedded slider table (count, names, resref lengths, TGA headers and sizes, pixel sanity: frame hollow, wedge
// taper, marker centred, description box filled with an outline and rounded corners), registration through EmbedTex with a fake engine, the pooled image getter, the label fill helper and layout helpers.
// Build/run: python3 scripts/run_tests.py slider_art
// Expected values are independent literals (512x64 = 131116 bytes, 32x128 = 16428 bytes, 256x128 = 131116 bytes: 18 header + w*h*4 pixels + 26 footer).
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
static int nNew = 0, nCtor = 0, nAdd = 0; static char g_ctorName[4][32]; static int g_nLoad = 0; static void* g_loadResult = nullptr;
static void* g_alloc[16]; static int g_nAlloc = 0;   // every fake-engine allocation, freed at the end (LeakSanitizer; the real engine owns these)
static void* track(void* p) { if (g_nAlloc < 16) g_alloc[g_nAlloc++] = p; return p; }
static void* fNew(unsigned n) { ++nNew; return track(calloc(1, n)); }
static void* fNewArr(unsigned n) { return track(malloc(n)); }
static void* fCtor(void* t, const char* a, const char*) { if (nCtor < 4) strncpy(g_ctorName[nCtor], a, 31); ++nCtor; return t; }
static void fAdd(void*, void*) { ++nAdd; }
static void fReset(void*) {}
static void* fLoad(const char*) { ++g_nLoad; return g_loadResult; }

static unsigned alphaAt(const embtex::Tex& t, int w, int x, int y) {   // y = 0 is the top row; TGA rows are stored bottom-up
    embtex::TgaInfo ti; embtex::tgaInspect(t.data, t.size, &ti);
    return t.data[ti.off + (((size_t)(ti.h - 1 - y)) * w + x) * 4 + 3];
}

int main() {
    // ---- table: four textures, fixed names and order, resrefs fit the 16-char limit ----
    KT_CHECK_EQ(sliderArtTextureCount(), 4);
    const embtex::Tex* t = sliderArtTextures();
    KT_CHECK(!strcmp(t[0].resref, "hse_sld_frame")); KT_CHECK(!strcmp(t[1].resref, "hse_sld_wedge")); KT_CHECK(!strcmp(t[2].resref, "hse_sld_mark")); KT_CHECK(!strcmp(t[3].resref, "hse_desc_box"));
    KT_CHECK(!strcmp(sliderArtResref(SA_Frame), "hse_sld_frame")); KT_CHECK(!strcmp(sliderArtResref(SA_Wedge), "hse_sld_wedge"));
    KT_CHECK(!strcmp(sliderArtResref(SA_Mark), "hse_sld_mark")); KT_CHECK(!strcmp(sliderArtResref(SA_Box), "hse_desc_box")); KT_CHECK(!strcmp(sliderArtResref(4), "")); KT_CHECK(!strcmp(sliderArtResref(-1), ""));
    for (int i = 0; i < 4; ++i) KT_CHECK(strlen(t[i].resref) > 0 && strlen(t[i].resref) <= 16);
    KT_CHECK_EQ(kSliderWideW, 512); KT_CHECK_EQ(kSliderWideH, 64); KT_CHECK_EQ(kSliderMarkW, 32); KT_CHECK_EQ(kSliderMarkH, 128);
    const int W[4] = { 512, 512, 32, 256 }, H[4] = { 64, 64, 128, 128 }; const unsigned SZ[4] = { 131116u, 131116u, 16428u, 131116u };
    for (int i = 0; i < 4; ++i) {
        KT_CHECK_EQ(t[i].size, SZ[i]);
        embtex::TgaInfo ti;
        KT_CHECK_EQ((int)embtex::tgaInspect(t[i].data, t[i].size, &ti), (int)embtex::TGA_OK);
        KT_CHECK_EQ(ti.w, W[i]); KT_CHECK_EQ(ti.h, H[i]); KT_CHECK_EQ(ti.off, 18u); KT_CHECK_EQ(ti.bytes, (unsigned)(W[i] * H[i] * 4));
        KT_CHECK_EQ(t[i].data[17], 0x08);                                        // 8 alpha bits, bottom-left origin
        KT_CHECK(!memcmp(t[i].data + t[i].size - 18, "TRUEVISION-XFILE.", 17));  // TGA 2.0 footer
        unsigned opaque = 0, clear = 0;
        for (int y = 0; y < H[i]; ++y) for (int x = 0; x < W[i]; ++x) { unsigned a = alphaAt(t[i], W[i], x, y); if (a == 255) ++opaque; else if (a < 16) ++clear; }
        KT_CHECK(opaque > 0);                              // non-empty alpha
        if (i != 3) KT_CHECK(clear > (unsigned)(W[i] * H[i]) / 4);   // (near-)transparent background: the slider parts are not filled rectangles
        else { KT_CHECK(opaque > (unsigned)(W[i] * H[i]) * 9 / 10); KT_CHECK(clear > 0); }   // the box is mostly solid, with only the rounded corners clear
    }

    // ---- frame (512x64, 8 px stroke, 24 px corner radius): hollow outline, all four edges drawn, corners rounded ----
    KT_CHECK_EQ(alphaAt(t[0], 512, 256, 0), 255u); KT_CHECK_EQ(alphaAt(t[0], 512, 256, 63), 255u);   // top and bottom edges
    KT_CHECK_EQ(alphaAt(t[0], 512, 0, 32), 255u); KT_CHECK_EQ(alphaAt(t[0], 512, 511, 32), 255u);    // left and right edges
    KT_CHECK_EQ(alphaAt(t[0], 512, 256, 32), 0u); KT_CHECK_EQ(alphaAt(t[0], 512, 80, 20), 0u);       // transparent inside
    KT_CHECK_EQ(alphaAt(t[0], 512, 0, 0), 0u); KT_CHECK_EQ(alphaAt(t[0], 512, 511, 63), 0u);         // rounded corners
    { const unsigned char* d = t[0].data + 18 + (size_t)(63 - 0) * 512 * 4 + 256 * 4;                // top edge pixel (row 0 is stored last): teal, G > R
      KT_CHECK(d[1] > d[2] && d[1] > d[0]); }

    // ---- wedge: thin at the left, tall at the right edge; per-column coverage never shrinks; 12 px empty margin on the left ----
    unsigned prev = 0, first = 0, last = 0;
    for (int x = 0; x < 512; ++x) {
        unsigned sum = 0; for (int y = 0; y < 64; ++y) sum += alphaAt(t[1], 512, x, y);
        if (x < 12) { KT_CHECK_EQ(sum, 0u); continue; }   // columns 12..511 carry the wedge (the tall end touches the right edge, so no right margin)
        KT_CHECK(sum >= prev); prev = sum;
        if (x == 32) first = sum;
        if (x == 500) last = sum;
    }
    KT_CHECK(first > 0 && last > 3 * first);                                  // clearly tapered
    KT_CHECK(alphaAt(t[1], 512, 500, 16) == 255u && alphaAt(t[1], 512, 32, 16) == 0u);   // tall end covers row 16, thin end does not
    KT_CHECK(alphaAt(t[1], 512, 511, 32) == 255u);                            // the tall end reaches the right edge
    for (int x = 0; x < 512; ++x) KT_CHECK_EQ(alphaAt(t[1], 512, x, 0), 0u);          // top and bottom rows are the frame's
    for (int x = 0; x < 512; ++x) KT_CHECK_EQ(alphaAt(t[1], 512, x, 63), 0u);

    // ---- marker (32x128): white, vertically and horizontally symmetric (centred), arrowheads at both ends ----
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 16; ++x) KT_CHECK_EQ(alphaAt(t[2], 32, x, y), alphaAt(t[2], 32, 31 - x, y));   // left/right mirror: centred on x = 16
        for (int x = 0; x < 32; ++x) KT_CHECK_EQ(alphaAt(t[2], 32, x, y), alphaAt(t[2], 32, x, 127 - y));  // top/bottom mirror: double arrow
    }
    KT_CHECK_EQ(alphaAt(t[2], 32, 12, 64), 255u); KT_CHECK_EQ(alphaAt(t[2], 32, 19, 64), 255u);   // 8 px bar in the middle (columns 12..19)
    KT_CHECK_EQ(alphaAt(t[2], 32, 11, 64), 0u); KT_CHECK_EQ(alphaAt(t[2], 32, 20, 64), 0u);
    KT_CHECK_EQ(alphaAt(t[2], 32, 4, 64), 0u); KT_CHECK_EQ(alphaAt(t[2], 32, 27, 64), 0u);
    KT_CHECK(alphaAt(t[2], 32, 2, 23) > 0 && alphaAt(t[2], 32, 29, 104) > 0);                     // arrowhead bases span the full width
    KT_CHECK_EQ(alphaAt(t[2], 32, 0, 0), 0u); KT_CHECK_EQ(alphaAt(t[2], 32, 8, 2), 0u);           // apex is narrow
    KT_CHECK_EQ(alphaAt(t[2], 32, 4, 12), 0u);                                                    // mid-head is narrower than the base
    { embtex::TgaInfo ti; embtex::tgaInspect(t[2].data, t[2].size, &ti);
      for (int p = 0; p < 32 * 128; ++p) { const unsigned char* d = t[2].data + ti.off + p * 4; if (d[3]) KT_CHECK(d[0] == 255 && d[1] == 255 && d[2] == 255); } }
    for (int i = 0; i < 3; ++i) KT_CHECK(memcmp(t[i].data, t[i + 1].data, t[i].size < t[i + 1].size ? t[i].size : t[i + 1].size) != 0);   // distinct images

    // ---- description box (256x128): opaque fill (0,17,11), 3 texel outline (0,105,81), corner radius 10; stored BGRA ----
    auto px = [&](int x, int y, unsigned* b, unsigned* g, unsigned* r, unsigned* a) {   // y = 0 is the top row
        embtex::TgaInfo ti; embtex::tgaInspect(t[3].data, t[3].size, &ti);
        const unsigned char* d = t[3].data + ti.off + (((size_t)(ti.h - 1 - y)) * 256 + x) * 4; *b = d[0]; *g = d[1]; *r = d[2]; *a = d[3]; };
    unsigned pb, pg, pr, pa;
    px(0, 0, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 0u); px(255, 127, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 0u);     // rounded corners
    px(255, 0, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 0u); px(0, 127, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 0u);
    px(128, 64, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 255u); KT_CHECK_EQ(pr, 0u); KT_CHECK_EQ(pg, 17u); KT_CHECK_EQ(pb, 11u);   // centre = fill
    px(128, 1, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 255u); KT_CHECK_EQ(pr, 0u); KT_CHECK_EQ(pg, 105u); KT_CHECK_EQ(pb, 81u);    // top edge = outline
    px(128, 126, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 255u); KT_CHECK_EQ(pg, 105u); KT_CHECK_EQ(pb, 81u);                       // bottom edge
    px(1, 64, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 255u); KT_CHECK_EQ(pg, 105u); KT_CHECK_EQ(pb, 81u);                          // left edge
    px(254, 64, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 255u); KT_CHECK_EQ(pg, 105u); KT_CHECK_EQ(pb, 81u);                        // right edge
    px(128, 5, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 255u); KT_CHECK_EQ(pg, 17u); KT_CHECK_EQ(pb, 11u);                          // just inside the outline = fill
    px(5, 64, &pb, &pg, &pr, &pa); KT_CHECK_EQ(pa, 255u); KT_CHECK_EQ(pg, 17u); KT_CHECK_EQ(pb, 11u);

    // ---- registration with a fake engine (Steam table is the default) ----
    static unsigned char pending[16] = {0};
    embtex::g_et.newFn = fNew; embtex::g_et.newArrFn = fNewArr; embtex::g_et.ctorFn = fCtor; embtex::g_et.listAddFn = fAdd; embtex::g_et.resetFn = fReset;
    embtex::g_et.pendingList = reinterpret_cast<uintptr_t>(pending);
    KT_CHECK(sliderArtEnsure());
    KT_CHECK_EQ(nNew, 4); KT_CHECK_EQ(nCtor, 4); KT_CHECK_EQ(nAdd, 4); KT_CHECK_EQ(embtex::slotCount(), 4);
    KT_CHECK(!strcmp(g_ctorName[0], "hse_sld_frame") && !strcmp(g_ctorName[1], "hse_sld_wedge") && !strcmp(g_ctorName[2], "hse_sld_mark") && !strcmp(g_ctorName[3], "hse_desc_box"));
    KT_CHECK(sliderArtEnsure()); KT_CHECK_EQ(nNew, 4);   // idempotent

    // ---- image getter: nullptr until the engine can resolve the name (not cached), then pooled per part ----
    guikit::g_gk.loadImage = fLoad; g_loadResult = nullptr;
    KT_CHECK(sliderArtImage(SA_Frame) == nullptr && sliderArtImage(SA_Wedge) == nullptr && sliderArtImage(SA_Mark) == nullptr && sliderArtImage(SA_Box) == nullptr);
    int before = g_nLoad;
    KT_CHECK(sliderArtImage(SA_Mark) == nullptr); KT_CHECK_EQ(g_nLoad, before + 1);   // a miss retries
    KT_CHECK(sliderArtImage(4) == nullptr && sliderArtImage(-1) == nullptr); KT_CHECK_EQ(g_nLoad, before + 1);   // out of range: no engine call
    // ---- sliderArtFill with no texture yet / no control: false, no write attempt ----
    KT_CHECK(!sliderArtFill(0x1000, SA_Frame)); KT_CHECK(!sliderArtApply(0x1000, 0x2000, 0x3000));
    g_loadResult = reinterpret_cast<void*>(0x4000);
    KT_CHECK(sliderArtImage(SA_Frame) == g_loadResult); KT_CHECK(sliderArtImage(SA_Wedge) == g_loadResult); KT_CHECK(sliderArtImage(SA_Mark) == g_loadResult); KT_CHECK(sliderArtImage(SA_Box) == g_loadResult);
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
