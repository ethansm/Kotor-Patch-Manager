// Host test of Gui2D.h (doc 20 conventions): engine calls replaced through gui2d::g_g2d, memory words in a MAP_32BIT arena.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#define GUI2D_HOST 1
#include "Gui2D.h"
#include "check.h"
#include <sys/mman.h>
#include <math.h>

using namespace gui2d;

static uint8_t* g_arena = nullptr;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// ---- fakes (captureless lambdas -> plain function pointers; state in globals) ----
struct PushCall { int x, y, w, h; const void* color; int flag; float alpha; };
static PushCall g_last; static int g_pushes = 0, g_pops = 0, g_begins = 0, g_ends = 0, g_flushes = 0, g_draws = 0, g_panelCalls = 0;
static int g_pushResult = 1; static float g_flushArg = -1; static float g_scale = 1.0f;
static int g_order[32]; static int g_nOrder = 0;            // 1 begin, 2 end, 3 push, 4 pop
static int g_panelR[4]; static uintptr_t g_panelSeen = 0;
static void* g_drawImg = nullptr; static int g_drawArgs[6]; static const float* g_drawRgb = nullptr; static float g_drawAlpha = 0;
static void note(int k) { if (g_nOrder < 32) g_order[g_nOrder++] = k; }
static void reset() { g_pushes = g_pops = g_begins = g_ends = g_flushes = g_draws = g_panelCalls = 0; g_nOrder = 0; g_pushResult = 1; g_last = {}; }

static void installFakes() {
    g_g2d.begin = [] { ++g_begins; note(1); };
    g_g2d.end = [] { ++g_ends; note(2); };
    g_g2d.push = [](int x, int y, int w, int h, const void* c, int f, float a) { ++g_pushes; note(3); g_last = { x, y, w, h, c, f, a }; return g_pushResult; };
    g_g2d.pop = [] { ++g_pops; note(4); };
    g_g2d.flush = [](float a) { ++g_flushes; g_flushArg = a; };
    g_g2d.hudScale = [] { return g_scale; };
    g_g2d.panelRect = [](uintptr_t p, int* r) { ++g_panelCalls; g_panelSeen = p; for (int i = 0; i < 4; ++i) r[i] = g_panelR[i]; };
    g_g2d.draw = [](void* img, int x, int y, int w, int h, int a5, int rot, const float* rgb, float al) {
        ++g_draws; g_drawImg = img; g_drawArgs[0] = x; g_drawArgs[1] = y; g_drawArgs[2] = w; g_drawArgs[3] = h; g_drawArgs[4] = a5; g_drawArgs[5] = rot;
        g_drawRgb = rgb; g_drawAlpha = al;
    };
}

int main() {
    void* m = mmap(nullptr, 1u << 12, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    uintptr_t base = reinterpret_cast<uintptr_t>(g_arena);
    installFakes();
    g_g2d.flushArgAddr = base; g_g2d.clipDepthAddr = base + 16;
    static const float kNoFill[4] = {}; g_g2d.clipNoFill = kNoFill;   // distinguishable from the engine constant

    // default engine address constants are what the header documents
    KT_CHECK_EQ((unsigned long long)AddrPushClipRect, 0x479460ull);
    KT_CHECK_EQ((unsigned long long)AddrClipNoFillColor, 0xA1BA3Cull);

    // ---- pushClip: default colour = no-fill, alpha passthrough, flag 0 ----
    {
        reset();
        int r = pushClip(10, 20, 30, 40);
        KT_CHECK_EQ((long long)r, 1ll); KT_CHECK_EQ((long long)g_pushes, 1ll);
        KT_CHECK_EQ((long long)g_last.x, 10ll); KT_CHECK_EQ((long long)g_last.y, 20ll);
        KT_CHECK_EQ((long long)g_last.w, 30ll); KT_CHECK_EQ((long long)g_last.h, 40ll);
        KT_CHECK(g_last.color == kNoFill); KT_CHECK(g_last.color == clipNoFill());
        KT_CHECK_EQ((long long)g_last.flag, 0ll); KT_CHECK(g_last.alpha == 1.0f);
        const float red[3] = { 1, 0, 0 };
        pushClip(1, 2, 3, 4, red, 0.25f);
        KT_CHECK(g_last.color == red); KT_CHECK(g_last.alpha == 0.25f);
        pushClip(1, 2, 3, 4, nullptr, 0.5f);                          // explicit null = default
        KT_CHECK(g_last.color == kNoFill); KT_CHECK(g_last.alpha == 0.5f);
        g_pushResult = 0; KT_CHECK_EQ((long long)pushClip(0, 0, 1, 1), 0ll);   // result passes through
        g_pushResult = 7; KT_CHECK_EQ((long long)pushClip(0, 0, 1, 1), 7ll);
        reset(); popClip(); KT_CHECK_EQ((long long)g_pops, 1ll);
    }

    // ---- fillRect: push then pop only on success ----
    {
        const float rgb[3] = { 0.2f, 0.4f, 0.6f };
        reset(); fillRect(5, 6, 7, 8, rgb, 0.75f);
        KT_CHECK_EQ((long long)g_pushes, 1ll); KT_CHECK_EQ((long long)g_pops, 1ll);
        KT_CHECK(g_last.color == rgb); KT_CHECK(g_last.alpha == 0.75f);
        KT_CHECK_EQ((long long)g_last.w, 7ll);
        KT_CHECK_EQ((long long)g_nOrder, 2ll); KT_CHECK_EQ((long long)g_order[0], 3ll); KT_CHECK_EQ((long long)g_order[1], 4ll);
        reset(); g_pushResult = 0; fillRect(5, 6, 7, 8, rgb, 1.0f);
        KT_CHECK_EQ((long long)g_pushes, 1ll); KT_CHECK_EQ((long long)g_pops, 0ll);
    }

    // ---- ClipScope ----
    {
        reset();
        { ClipScope c(1, 2, 3, 4); KT_CHECK(static_cast<bool>(c)); KT_CHECK_EQ((long long)g_pops, 0ll); KT_CHECK(g_last.color == kNoFill); }
        KT_CHECK_EQ((long long)g_pushes, 1ll); KT_CHECK_EQ((long long)g_pops, 1ll);
        reset(); g_pushResult = 0;
        { ClipScope c(1, 2, 3, 4); KT_CHECK(!c); if (c) KT_CHECK(false); }
        KT_CHECK_EQ((long long)g_pushes, 1ll); KT_CHECK_EQ((long long)g_pops, 0ll);
        reset();
        const float green[3] = { 0, 1, 0 };
        { ClipScope outer(0, 0, 100, 100, green, 0.5f); { ClipScope inner(1, 1, 2, 2); } KT_CHECK_EQ((long long)g_pops, 1ll); }
        KT_CHECK_EQ((long long)g_pushes, 2ll); KT_CHECK_EQ((long long)g_pops, 2ll);
        KT_CHECK_EQ((long long)g_order[0], 3ll); KT_CHECK_EQ((long long)g_order[3], 4ll);
        // if (ClipScope c{...}) idiom
        reset(); int drawn = 0;
        if (ClipScope c{ 0, 0, 5, 5 }) ++drawn;
        KT_CHECK_EQ((long long)drawn, 1ll); KT_CHECK_EQ((long long)g_pops, 1ll);
    }

    // ---- Gui2DScope: begin/end exactly once, end after the body ----
    {
        reset();
        { Gui2DScope s; KT_CHECK_EQ((long long)g_begins, 1ll); KT_CHECK_EQ((long long)g_ends, 0ll); pushClip(0, 0, 1, 1); popClip(); }
        KT_CHECK_EQ((long long)g_begins, 1ll); KT_CHECK_EQ((long long)g_ends, 1ll);
        KT_CHECK_EQ((long long)g_nOrder, 4ll);
        KT_CHECK_EQ((long long)g_order[0], 1ll); KT_CHECK_EQ((long long)g_order[3], 2ll);
        reset();
        { Gui2DScope s; ClipScope c(1, 1, 1, 1); }                    // clip pops before the batch closes
        KT_CHECK_EQ((long long)g_order[0], 1ll); KT_CHECK_EQ((long long)g_order[1], 3ll);
        KT_CHECK_EQ((long long)g_order[2], 4ll); KT_CHECK_EQ((long long)g_order[3], 2ll);
        KT_CHECK_EQ((long long)g_begins, 1ll); KT_CHECK_EQ((long long)g_ends, 1ll);
    }

    // ---- hudScale clamps ----
    {
        const float good[] = { 0.11f, 0.5f, 1.0f, 1.5f, 2.0f, 9.99f };
        for (float s : good) { g_scale = s; kt::Check("hudScale passes a sane value", hudScale() == s); }
        const float bad[] = { 0.0f, -1.0f, 0.1f, 0.05f, 10.0f, 10.5f, 1e9f, -1e9f, INFINITY, -INFINITY, NAN };
        for (float s : bad) { g_scale = s; kt::Check("hudScale clamps to 1.0", hudScale() == 1.0f); }
        g_scale = 1.0f;
    }

    // ---- flushArg / flush / drawImage ----
    {
        at<float>(base, 0) = 0.375f; KT_CHECK(flushArg() == 0.375f);
        g_g2d.flushArgAddr = 0x20; KT_CHECK(flushArg() == 0.0f);      // unreadable -> 0
        g_g2d.flushArgAddr = base;
        reset(); flush(flushArg()); KT_CHECK_EQ((long long)g_flushes, 1ll); KT_CHECK(g_flushArg == 0.375f);
        reset(); int img; const float rgb[3] = { 1, 1, 1 };
        drawImage(&img, 1, 2, 3, 4, 0x55, rgb, 0.5f);
        KT_CHECK_EQ((long long)g_draws, 1ll); KT_CHECK(g_drawImg == &img);
        KT_CHECK_EQ((long long)g_drawArgs[0], 1ll); KT_CHECK_EQ((long long)g_drawArgs[3], 4ll);
        KT_CHECK_EQ((long long)g_drawArgs[4], 0ll); KT_CHECK_EQ((long long)g_drawArgs[5], 0x55ll);
        KT_CHECK(g_drawRgb == rgb); KT_CHECK(g_drawAlpha == 0.5f);
    }

    // ---- clip depth / inMenuFrame: low 16 bits only ----
    {
        struct { unsigned v; bool menu; } cases[] = {
            { 0u, false }, { 1u, true }, { 0xFFFFu, true }, { 0x10000u, false }, { 0xABCD0000u, false },
            { 0xABCD0001u, true }, { 0xFFFF0000u, false }, { 0xFFFFFFFFu, true }, { 0x8000u, true } };
        for (auto& c : cases) {
            at<unsigned>(base, 16) = c.v;
            char w[48]; snprintf(w, sizeof w, "inMenuFrame(0x%08x)", c.v);
            kt::Check(w, inMenuFrame() == c.menu);
            KT_CHECK_EQ((long long)clipDepthRaw(), (long long)(int)c.v);
        }
        g_g2d.clipDepthAddr = 0x20;                                   // unreadable -> raw 0 -> HUD frame
        KT_CHECK_EQ((long long)clipDepthRaw(), 0ll); KT_CHECK(!inMenuFrame());
        g_g2d.clipDepthAddr = base + 16;
    }

    // ---- panelRect validation ----
    {
        struct { int w, h; bool ok; } cases[] = {
            { 100, 50, true }, { 1, 1, true }, { 8192, 8192, true }, { 8193, 50, false }, { 50, 8193, false },
            { 0, 50, false }, { 50, 0, false }, { -5, 50, false }, { 50, -5, false }, { 0, 0, false } };
        for (auto& c : cases) {
            g_panelR[0] = 11; g_panelR[1] = 22; g_panelR[2] = c.w; g_panelR[3] = c.h;
            int r[4] = { 9, 9, 9, 9 }; reset();
            bool ok = panelRect(0x1234, r);
            char w[48]; snprintf(w, sizeof w, "panelRect %dx%d", c.w, c.h);
            kt::Check(w, ok == c.ok);
            KT_CHECK_EQ((long long)g_panelCalls, 1ll); KT_CHECK_EQ((unsigned long long)g_panelSeen, 0x1234ull);
            KT_CHECK_EQ((long long)r[0], 11ll); KT_CHECK_EQ((long long)r[1], 22ll);   // rect is returned even when rejected
        }
        // the out array is zeroed before the engine call: an engine that writes nothing is rejected
        auto saved = g_g2d.panelRect;
        g_g2d.panelRect = [](uintptr_t, int*) {};
        int r[4] = { 5, 5, 5, 5 }; KT_CHECK(!panelRect(1, r));
        KT_CHECK_EQ((long long)r[0], 0ll); KT_CHECK_EQ((long long)r[3], 0ll);
        g_g2d.panelRect = saved;
    }

    // ---- vfillClip: bottom-anchored fill, frac clamped, NaN = 0, round half up ----
    {
        Rect r{ 10, 20, 30, 40 };                                     // bottom edge y+h = 60
        auto chk = [&](const char* nm, float f, int wantH) {
            Rect c = vfillClip(r, f);
            char w[64]; snprintf(w, sizeof w, "vfillClip %s", nm);
            kt::Check(w, c.h == wantH && c.x == 10 && c.w == 30 && c.y + c.h == 60 && c.y == 20 + 40 - wantH);
        };
        chk("0", 0.0f, 0); chk("1", 1.0f, 40); chk("0.5", 0.5f, 20); chk("0.25", 0.25f, 10);
        chk(">1", 2.5f, 40); chk("<0", -0.5f, 0); chk("nan", NAN, 0); chk("inf", INFINITY, 40); chk("-inf", -INFINITY, 0);
        chk("tiny", 0.001f, 0); chk("just-below-1", 0.999f, 40);
        // odd heights: round half up (int(h*f + 0.5f))
        Rect o{ 0, 0, 5, 7 };
        KT_CHECK_EQ((long long)vfillClip(o, 0.5f).h, 4ll);            // 3.5 -> 4
        KT_CHECK_EQ((long long)vfillClip(o, 0.0f).h, 0ll);
        KT_CHECK_EQ((long long)vfillClip(o, 1.0f).h, 7ll);
        KT_CHECK_EQ((long long)vfillClip(o, 0.25f).h, 2ll);           // 1.75 -> 2
        KT_CHECK_EQ((long long)vfillClip(o, 0.2f).h, 1ll);            // 1.4 -> 1
        KT_CHECK_EQ((long long)vfillClip(o, 0.5f).y, 3ll);            // bottom edge 7
        Rect one{ 3, 4, 2, 1 };
        KT_CHECK_EQ((long long)vfillClip(one, 0.5f).h, 1ll);          // 0.5 -> 1
        KT_CHECK_EQ((long long)vfillClip(one, 0.49f).h, 0ll);
        Rect z{ 1, 2, 3, 0 };                                         // empty rect stays empty
        KT_CHECK_EQ((long long)vfillClip(z, 1.0f).h, 0ll); KT_CHECK_EQ((long long)vfillClip(z, 1.0f).y, 2ll);
        // matches buff_duration_hud brightPx (int(size*ratio+0.5f)) and shield_bar's gauge (round(h*cur/mx)) on the same inputs
        for (int h = 1; h <= 24; ++h) for (int n = 0; n <= 10; ++n) {
            float f = float(n) / 10.0f;
            KT_CHECK_EQ((long long)vfillClip({ 0, 0, 1, h }, f).h, (long long)int(h * f + 0.5f));
        }
    }

    // ---- drawVFill: clip on the bottom slice, image drawn offset so the slice shows the bottom part ----
    {
        static const float rgb[3] = { 1, 0.5f, 0.25f };
        int img; Rect r{ 10, 20, 30, 40 };
        reset(); g_draws = 0;
        KT_CHECK(drawVFill(&img, r, 0.5f, rgb, 0.75f));
        KT_CHECK_EQ((long long)g_pushes, 1ll); KT_CHECK_EQ((long long)g_pops, 1ll);
        KT_CHECK_EQ((long long)g_last.x, 10ll); KT_CHECK_EQ((long long)g_last.y, 40ll);
        KT_CHECK_EQ((long long)g_last.w, 30ll); KT_CHECK_EQ((long long)g_last.h, 20ll);
        KT_CHECK(g_last.color == clipNoFill());
        KT_CHECK_EQ((long long)g_draws, 1ll); KT_CHECK(g_drawImg == &img);
        KT_CHECK_EQ((long long)g_drawArgs[0], 0ll); KT_CHECK_EQ((long long)g_drawArgs[1], -20ll);
        KT_CHECK_EQ((long long)g_drawArgs[2], 30ll); KT_CHECK_EQ((long long)g_drawArgs[3], 40ll);
        KT_CHECK(g_drawRgb == rgb); KT_CHECK(g_drawAlpha == 0.75f);
        KT_CHECK_EQ((long long)g_order[0], 3ll); KT_CHECK_EQ((long long)g_order[1], 4ll);
        // full fill: offset 0
        reset(); g_draws = 0; KT_CHECK(drawVFill(&img, r, 1.0f, rgb, 1.0f));
        KT_CHECK_EQ((long long)g_drawArgs[1], 0ll); KT_CHECK_EQ((long long)g_last.h, 40ll);
        KT_CHECK_EQ((long long)g_drawArgs[4], 0ll); KT_CHECK_EQ((long long)g_drawArgs[5], 0ll);   // a5 and rotation bits stay 0
        // 1 px high / 1 px wide slices still draw
        reset(); g_draws = 0; KT_CHECK(drawVFill(&img, { 0, 0, 30, 40 }, 0.025f, rgb, 1.0f));
        KT_CHECK_EQ((long long)g_last.h, 1ll); KT_CHECK_EQ((long long)g_last.y, 39ll); KT_CHECK_EQ((long long)g_draws, 1ll);
        KT_CHECK_EQ((long long)g_drawArgs[1], -39ll);
        reset(); g_draws = 0; KT_CHECK(drawVFill(&img, { 5, 0, 1, 40 }, 1.0f, rgb, 1.0f));
        KT_CHECK_EQ((long long)g_last.w, 1ll); KT_CHECK_EQ((long long)g_draws, 1ll);
        // empty fill: nothing pushed or drawn
        reset(); g_draws = 0; KT_CHECK(!drawVFill(&img, r, 0.0f, rgb, 1.0f));
        KT_CHECK_EQ((long long)g_pushes, 0ll); KT_CHECK_EQ((long long)g_draws, 0ll);
        reset(); g_draws = 0; KT_CHECK(!drawVFill(&img, { 0, 0, 0, 10 }, 1.0f, rgb, 1.0f));   // zero width
        KT_CHECK_EQ((long long)g_pushes, 0ll);
        // refused clip push: no draw, no pop
        reset(); g_draws = 0; g_pushResult = 0;
        KT_CHECK(!drawVFill(&img, r, 0.5f, rgb, 1.0f));
        KT_CHECK_EQ((long long)g_pushes, 1ll); KT_CHECK_EQ((long long)g_pops, 0ll); KT_CHECK_EQ((long long)g_draws, 0ll);
        reset();
    }

    return kt::Report("gui2d_test");
}
