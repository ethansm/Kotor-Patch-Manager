// Host test of HudKit.h: the five kit textures (names, order, 32-bit TGA headers, dimensions, colour/alpha pixels), the colour constants,
// and the image getters (null without a loader, retried while missing, cached once found). Expected values are independent literals.
#ifndef EMBTEX_HOST
#define EMBTEX_HOST 1
#endif
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "HudKit.h"
#include "check.h"
#include <stdlib.h>
#include <string.h>

using namespace hudkit;

// Pixel (x, y) of the embedded TGA as {b, g, r, a} (stored BGRA, rows as written).
static void px(int id, int x, int y, int out[4]) {
    const embtex::Tex& t = textures()[id]; embtex::TgaInfo ti;
    embtex::tgaInspect(t.data, t.size, &ti);
    const unsigned char* p = t.data + ti.off + (static_cast<size_t>(y) * ti.w + x) * 4;
    for (int i = 0; i < 4; ++i) out[i] = p[i];
}
static int g_loads = 0;
static char g_lastName[32], g_tok[8];
static void* fakeLoad(const char* name) { ++g_loads; strncpy(g_lastName, name, 31); return strcmp(name, "kit_cancel_hi") == 0 ? nullptr : &g_tok[g_loads % 8]; }
static int g_flakyCalls = 0;   // SH-3a-05 loader: refuses its first call, hands out a handle afterwards
static void* flakyLoad(const char*) { ++g_flakyCalls; return g_flakyCalls == 1 ? nullptr : &g_tok[0]; }
static void* nullLoad(const char*) { ++g_loads; return nullptr; }

int main() {
    // ---- names + order ----
    KT_CHECK_EQ((int)Count, 5);
    KT_CHECK(!strcmp(resref(Frame), "kit_frame")); KT_CHECK(!strcmp(resref(FrameHi), "kit_frame_hi"));
    KT_CHECK(!strcmp(resref(Cancel), "kit_cancel")); KT_CHECK(!strcmp(resref(CancelHi), "kit_cancel_hi"));
    KT_CHECK(!strcmp(resref(Disc), "kit_disc")); KT_CHECK(!strcmp(resref(5), "")); KT_CHECK(!strcmp(resref(-1), ""));
    KT_CHECK(textures()[Disc].resref == resref(Disc));
    // ---- headers + dimensions: uncompressed 32-bit TGA, 64x64 slots, 48x48 disc, exact byte size ----
    for (int id = 0; id < Count; ++id) {
        const embtex::Tex& t = textures()[id]; embtex::TgaInfo ti;
        KT_CHECK_EQ((int)embtex::tgaInspect(t.data, t.size, &ti), (int)embtex::TGA_OK);
        const int side = id == Disc ? 48 : 64;
        KT_CHECK_EQ(ti.w, side); KT_CHECK_EQ(ti.h, side); KT_CHECK_EQ((int)ti.off, 18);
        KT_CHECK_EQ((int)t.size, 18 + side * side * 4 + 26);   // header + pixels + 26 B TGA footer (PIL): 16428 slot tiles, 9260 disc
        KT_CHECK_EQ((int)t.data[2], 2); KT_CHECK_EQ((int)t.data[16], 32);
    }
    // ---- constants ----
    KT_CHECK(kBorder.r == 0 && kBorder.g == 44 && kBorder.b == 26);
    KT_CHECK(kHi.r == 208 && kHi.g == 204 && kHi.b == 170);
    KT_CHECK(kAccent.r == 10 && kAccent.g == 96 && kAccent.b == 80);
    KT_CHECK_EQ(kFillAlpha, 175);
    // ---- pixels agree with the constants (frame border mid-edge 1 px in, rounded corner clear, centre fill) ----
    { int p[4]; px(Frame, 32, 1, p);   KT_CHECK(abs(p[2] - kBorder.r) <= 12 && abs(p[1] - kBorder.g) <= 12 && abs(p[0] - kBorder.b) <= 12 && p[3] == 255); }
    { int p[4]; px(FrameHi, 32, 1, p); KT_CHECK(abs(p[2] - kHi.r) <= 12 && abs(p[1] - kHi.g) <= 12 && abs(p[0] - kHi.b) <= 12 && p[3] == 255); }
    { int p[4]; px(CancelHi, 32, 1, p); KT_CHECK(abs(p[2] - kHi.r) <= 12 && p[3] == 255); }
    { int p[4]; px(Frame, 0, 0, p);    KT_CHECK_EQ(p[3], 0); }
    { int p[4]; px(Frame, 32, 14, p);  KT_CHECK(abs(p[3] - kFillAlpha) <= 2); }
    { int p[4]; px(Cancel, 32, 32, p); KT_CHECK_EQ(p[3], 255); KT_CHECK(p[1] > p[2] && p[0] > p[2]); }   // X glyph: opaque mint/teal
    { int p[4]; px(Disc, 24, 24, p);   KT_CHECK(p[0] == 0 && p[1] == 0 && p[2] == 0 && abs(p[3] - kFillAlpha) <= 1); }   // disc fill
    { int p[4]; px(Disc, 24, 3, p);    KT_CHECK(abs(p[2] - kAccent.r) <= 6 && abs(p[1] - kAccent.g) <= 6 && abs(p[0] - kAccent.b) <= 6 && p[3] == 255); }   // disc rim (top, radius 21..19)
    { int p[4]; px(Disc, 0, 0, p);     KT_CHECK_EQ(p[3], 0); }
    // ---- getters: null without a loader (nothing pooled); cached once found ----
    guikit::g_gk.loadImage = nullptr;   // the default table points at engine addresses
    KT_CHECK(frame(false) == nullptr && frame(true) == nullptr && cancel(false) == nullptr && disc() == nullptr);
    KT_CHECK(image(-1) == nullptr && image(Count) == nullptr);
    guikit::g_gk.loadImage = fakeLoad; g_loads = 0;
    void* f = frame(false);
    KT_CHECK(f != nullptr); KT_CHECK(!strcmp(g_lastName, "kit_frame"));
    KT_CHECK(frame(false) == f);   // cached (and the guikit pool answers the repeat)
    KT_CHECK(frame(true) != nullptr && cancel(false) != nullptr && disc() != nullptr);
    KT_CHECK(cancel(true) == nullptr);   // this loader refuses kit_cancel_hi: nullptr, caller falls back
    KT_CHECK(g_loads >= 4);
    // ---- null then valid (SH-3a-05): kit_cancel_hi was refused above, yet a failed load is not cached, so the next image() asks again ----
    guikit::g_gk.loadImage = flakyLoad; g_flakyCalls = 0;
    KT_CHECK(cancel(true) == nullptr && g_flakyCalls == 1);   // call 1: the loader runs (nothing was pooled for the earlier refusal) and refuses
    void* h = cancel(true);                                   // call 2: the loader runs again and returns a handle
    KT_CHECK(h != nullptr && g_flakyCalls == 2);
    KT_CHECK(cancel(true) == h && g_flakyCalls == 2);         // found: cached, no third load
    // ---- imageForRetry out-params: a null load reports loaded but adds no pool entry; a handle is pooled once; imageFor still pools null ----
    {
        int pc0 = -1, pc1 = -1; bool ld = false;
        guikit::imageFor(nullptr, nullptr, &pc0);             // no lookup: reports the pool count only
        guikit::g_gk.loadImage = nullLoad; g_loads = 0;
        KT_CHECK(guikit::imageForRetry("kit_probe_a", &ld, &pc1) == nullptr);
        KT_CHECK(ld); KT_CHECK_EQ(pc1, pc0); KT_CHECK_EQ(g_loads, 1);
        KT_CHECK(guikit::imageForRetry("KIT_PROBE_A", &ld, &pc1) == nullptr);   // case-insensitive, asks again
        KT_CHECK(ld); KT_CHECK_EQ(pc1, pc0); KT_CHECK_EQ(g_loads, 2);
        guikit::g_gk.loadImage = fakeLoad; g_loads = 0;
        void* p = guikit::imageForRetry("kit_probe_a", &ld, &pc1);              // texture appears: pooled now
        KT_CHECK(p != nullptr && ld); KT_CHECK_EQ(pc1, pc0 + 1); KT_CHECK_EQ(g_loads, 1);
        KT_CHECK(guikit::imageForRetry("kit_probe_a", &ld, &pc1) == p && !ld); KT_CHECK_EQ(g_loads, 1);
        KT_CHECK(guikit::imageFor("kit_probe_a") == p); KT_CHECK_EQ(g_loads, 1);   // the shared pool answers imageFor too
        guikit::g_gk.loadImage = nullLoad; g_loads = 0;
        KT_CHECK(guikit::imageFor("kit_probe_b", &ld, &pc1) == nullptr); KT_CHECK_EQ(pc1, pc0 + 2);   // imageFor keeps pooling a null ...
        KT_CHECK(guikit::imageFor("kit_probe_b", &ld, &pc1) == nullptr && !ld); KT_CHECK_EQ(g_loads, 1);   // ... and does not ask again
    }
    guikit::g_gk.loadImage = nullptr;
    return kt::Report("hudkit_test");
}
