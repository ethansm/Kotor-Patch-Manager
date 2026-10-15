// Host test of EmbedTex.h: create path, upload / reset / Missing-rebuild heal state machine, 20-refill give-up, bad-TGA rejection and the
// Steam-only gate. Fake engine = host heap buffers; the "engine texture tick / GL reset" is simulated by writing the object's fields directly.
// Expected values are independent literals.
#ifndef EMBTEX_HOST
#define EMBTEX_HOST 1
#endif
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "EmbedTex.h"
#include "check.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

using namespace embtex;
static int nNew = 0, nNewArr = 0, nCtor = 0, nAdd = 0, nReset = 0; static void* lastAddList = nullptr; static const char* lastCtorName = nullptr;
static void* g_alloc[512]; static int g_nAlloc = 0;   // every fake-engine allocation, freed at the end (LeakSanitizer; the real engine owns these)
static void* track(void* p) { if (g_nAlloc < 512) g_alloc[g_nAlloc++] = p; return p; }
static void* fNew(unsigned n) { ++nNew; return track(calloc(1, n)); }
static void* fNewArr(unsigned n) { ++nNewArr; return track(malloc(n)); }
static void* fNoMem(unsigned) { ++nNewArr; return nullptr; }   // engine operator new[] failure: fill() logs and returns false
static void* fNewFail(unsigned) { ++nNew; return nullptr; }     // the engine cannot allocate the texture object
static void* fCtor(void* t, const char* a, const char*) { ++nCtor; lastCtorName = a; (void)t; return t; }
static void fAdd(void* list, void*) { ++nAdd; lastAddList = list; }
static void fReset(void* b) {   // the engine's Reset: pixels and GL name freed, not constructed
    ++nReset; unsigned char* o = static_cast<unsigned char*>(b);
    *reinterpret_cast<void**>(o + Tex_Pixels) = nullptr; *reinterpret_cast<int*>(o + Tex_Uploaded) = 0; o[Tex_Constructed] = 0; o[Tex_Missing] = 0;
}
static char g_lines[64][256]; static int g_nLines = 0;
static void fLog(const char* fmt, ...) { va_list ap; va_start(ap, fmt); if (g_nLines < 64) vsnprintf(g_lines[g_nLines++], 256, fmt, ap); va_end(ap); }
static int countLines(const char* sub) { int c = 0; for (int i = 0; i < g_nLines; ++i) if (strstr(g_lines[i], sub)) ++c; return c; }
static void resetCounters() { nNew = nNewArr = nCtor = nAdd = nReset = 0; g_nLines = 0; }

// 2x2 uncompressed 32-bit TGA: header 18 bytes, then 4 BGRA pixels.
static unsigned char kTga[18 + 16] = {0,0,2, 0,0,0,0,0, 0,0,0,0, 2,0, 2,0, 32,0,
    1,2,3,4,  5,6,7,8,  9,10,11,12,  13,14,15,16};
static unsigned char* B(const Slot& s) { return s.basic; }
template <typename T> static T& at(unsigned char* b, int off) { return *reinterpret_cast<T*>(b + off); }
// The texture tick uploaded the pixels: buffer freed (pixels NULL), GL name set.
static void simUpload(unsigned char* b) { at<void*>(b, Tex_Pixels) = nullptr; at<int>(b, Tex_Uploaded) = 7; }
// A GL reset: pixels freed, not constructed, GL name cleared.
static void simGlReset(unsigned char* b) { at<void*>(b, Tex_Pixels) = nullptr; at<int>(b, Tex_Uploaded) = 0; b[Tex_Constructed] = 0; }

int main() {
    static unsigned char pending[16] = {0}; at<int>(pending, 4) = 3;
    g_et.newFn = fNew; g_et.newArrFn = fNewArr; g_et.ctorFn = fCtor; g_et.listAddFn = fAdd; g_et.resetFn = fReset; g_et.pendingList = reinterpret_cast<uintptr_t>(pending);
    g_log = fLog;

    // ---- constants (Steam decomp, copied from the hotbar) ----
    KT_CHECK_EQ(ObjSize, 0xF4); KT_CHECK_EQ(Tex_Pixels, 0x44); KT_CHECK_EQ(Tex_Uploaded, 0x58); KT_CHECK_EQ(Tex_MipBytes, 0x5C); KT_CHECK_EQ(Tex_W, 0x68);
    KT_CHECK_EQ(Tex_H, 0x6C); KT_CHECK_EQ(Tex_Bpp, 0x74); KT_CHECK_EQ(Tex_Format, 0x78); KT_CHECK_EQ(Tex_Cube, 0xDF); KT_CHECK_EQ(Tex_Constructed, 0xE4);
    KT_CHECK_EQ(Tex_Missing, 0xE5); KT_CHECK_EQ(Tex_FromRes, 0xE7); KT_CHECK_EQ(Tex_Tpc, 0xE9); KT_CHECK_EQ(MaxTgaDim, 4096); KT_CHECK_EQ(MaxHeals, 20);
    KT_CHECK(supported());

    // ---- pure TGA helpers ----
    { TgaInfo ti; KT_CHECK_EQ((int)tgaInspect(kTga, sizeof kTga, &ti), (int)TGA_OK); KT_CHECK_EQ(ti.w, 2); KT_CHECK_EQ(ti.h, 2); KT_CHECK_EQ(ti.off, 18u); KT_CHECK_EQ(ti.bytes, 16u);
      unsigned char out[16]; tgaBgraToRgba(kTga, ti, out);
      KT_CHECK_EQ(out[0], 3); KT_CHECK_EQ(out[1], 2); KT_CHECK_EQ(out[2], 1); KT_CHECK_EQ(out[3], 4); KT_CHECK_EQ(out[12], 15); KT_CHECK_EQ(out[14], 13); KT_CHECK_EQ(out[15], 16); }

    // ---- create path ----
    const Tex set[1] = {{"emb_a", kTga, sizeof kTga}};
    KT_CHECK(ensure(set, 1));
    KT_CHECK_EQ(nNew, 1); KT_CHECK_EQ(nCtor, 1); KT_CHECK_EQ(nNewArr, 1); KT_CHECK_EQ(nAdd, 1); KT_CHECK(!strcmp(lastCtorName, "emb_a"));
    KT_CHECK(lastAddList == reinterpret_cast<void*>(pending));
    KT_CHECK_EQ(slotCount(), 1);
    unsigned char* b = B(slots()[0]);
    KT_CHECK(b != nullptr); KT_CHECK_EQ(at<int>(b, Tex_W), 2); KT_CHECK_EQ(at<int>(b, Tex_H), 2); KT_CHECK_EQ(at<int>(b, Tex_Bpp), 4);
    KT_CHECK_EQ(b[Tex_Constructed], 1); KT_CHECK_EQ(b[Tex_Missing], 0); KT_CHECK_EQ(b[Tex_FromRes], 0); KT_CHECK_EQ(b[Tex_Tpc], 0); KT_CHECK_EQ(b[Tex_Cube], 0);
    KT_CHECK_EQ(at<unsigned char*>(b, Tex_Pixels)[0], 3);   // BGRA -> RGBA
    KT_CHECK_EQ(countLines("tex emb_a: created"), 1); KT_CHECK_EQ(countLines("2x2 RGBA queued"), 1); KT_CHECK_EQ(countLines("pending 3"), 1);
    KT_CHECK(ensure(set, 1)); KT_CHECK_EQ(nNew, 1); KT_CHECK_EQ(nAdd, 1);   // idempotent

    // ---- healthy: pixels pending (not yet uploaded) -> no action is needed until the engine acts; uploaded -> healthy ----
    heal(); KT_CHECK_EQ(nAdd, 1); KT_CHECK_EQ(nReset, 0);   // built + pixels present: neither branch
    simUpload(b); heal(); KT_CHECK_EQ(nAdd, 1); KT_CHECK_EQ(nReset, 0); KT_CHECK_EQ(slots()[0].heals, 0);

    // ---- engine GL reset -> refill ----
    simGlReset(b); heal();
    KT_CHECK_EQ(nAdd, 2); KT_CHECK_EQ(nReset, 0); KT_CHECK_EQ(slots()[0].heals, 1); KT_CHECK_EQ(b[Tex_Constructed], 1); KT_CHECK(at<void*>(b, Tex_Pixels) != nullptr);
    KT_CHECK_EQ(countLines("tex emb_a: refilled after a texture reset"), 1);
    simUpload(b); heal(); KT_CHECK_EQ(slots()[0].heals, 0); KT_CHECK_EQ(nAdd, 2);   // healthy again clears the counter

    // ---- engine rebuilt first, resource lookup missed our name -> Reset + refill ----
    at<void*>(b, Tex_Pixels) = nullptr; at<int>(b, Tex_Uploaded) = 0; b[Tex_Constructed] = 1; b[Tex_Missing] = 1; heal();
    KT_CHECK_EQ(nReset, 1); KT_CHECK_EQ(nAdd, 3); KT_CHECK_EQ(b[Tex_Missing], 0); KT_CHECK_EQ(b[Tex_Constructed], 1); KT_CHECK_EQ(slots()[0].heals, 1);
    KT_CHECK_EQ(countLines("tex emb_a: rebuilt without our pixels"), 1); KT_CHECK_EQ(countLines("tex emb_a: refilled after a missing rebuild"), 1);
    simUpload(b); heal(); KT_CHECK_EQ(slots()[0].heals, 0);

    // ---- width changed, Missing=0: heal takes the w != s.w arm -> Reset + refill (the Missing=1 case above never reaches it) ----
    clearForTest(); resetCounters();
    KT_CHECK(ensure(set, 1)); b = B(slots()[0]); KT_CHECK_EQ(nAdd, 1);
    simUpload(b); heal(); KT_CHECK_EQ(nReset, 0);
    at<void*>(b, Tex_Pixels) = nullptr; at<int>(b, Tex_Uploaded) = 0; b[Tex_Constructed] = 1; b[Tex_Missing] = 0; at<int>(b, Tex_W) = 3; heal();
    KT_CHECK_EQ(nReset, 1); KT_CHECK_EQ(nAdd, 2); KT_CHECK_EQ(slots()[0].heals, 1); KT_CHECK_EQ(at<int>(b, Tex_W), 2); KT_CHECK_EQ(slots()[0].w, 2);
    KT_CHECK_EQ(b[Tex_Missing], 0); KT_CHECK_EQ(b[Tex_Constructed], 1); KT_CHECK(at<void*>(b, Tex_Pixels) != nullptr);
    KT_CHECK_EQ(countLines("tex emb_a: rebuilt without our pixels (missing=0 w=3)"), 1); KT_CHECK_EQ(countLines("tex emb_a: refilled after a missing rebuild"), 1);
    simUpload(b); heal(); KT_CHECK_EQ(slots()[0].heals, 0);   // healthy again re-arms the counter

    // ---- give-up after 20 refills: the 21st is not attempted, the give-up line appears once ----
    clearForTest(); resetCounters();
    KT_CHECK(ensure(set, 1)); b = B(slots()[0]); KT_CHECK_EQ(nAdd, 1);
    for (int i = 0; i < 25; ++i) { simGlReset(b); heal(); }
    KT_CHECK_EQ(nAdd, 1 + MaxHeals); KT_CHECK_EQ(slots()[0].heals, MaxHeals); KT_CHECK_EQ(countLines("tex emb_a: refilled after a texture reset"), MaxHeals);
    KT_CHECK_EQ(countLines("tex emb_a: giving up after 20 refill attempts"), 1);
    b[Tex_Constructed] = 1; simUpload(b); heal(); KT_CHECK_EQ(slots()[0].heals, 0);   // a late successful upload re-arms the counter

    // ---- 20th refill succeeds: the cap still engages, so the line stays, worded as attempts and reporting ok=1 ----
    clearForTest(); resetCounters();
    KT_CHECK(ensure(set, 1)); b = B(slots()[0]);
    for (int i = 0; i < MaxHeals - 1; ++i) { simGlReset(b); heal(); }
    KT_CHECK_EQ(slots()[0].heals, MaxHeals - 1); KT_CHECK_EQ(countLines("giving up"), 0);
    simGlReset(b); heal();   // 20th attempt: fill succeeds
    KT_CHECK_EQ(slots()[0].heals, MaxHeals); KT_CHECK_EQ(countLines("tex emb_a: giving up after 20 refill attempts"), 1); KT_CHECK_EQ(countLines("last fill ok=1"), 1);
    KT_CHECK_EQ(countLines("art missing"), 0);   // the 20th fill worked: the line must not claim the art is missing
    simUpload(b); heal(); KT_CHECK_EQ(slots()[0].heals, 0);   // the upload is observed: counter re-arms

    // ---- 20th refill fails (engine new[] returns NULL): the give-up line reports last fill ok=0 ----
    clearForTest(); resetCounters();
    KT_CHECK(ensure(set, 1)); b = B(slots()[0]);
    { auto svArr = g_et.newArrFn; g_et.newArrFn = fNoMem;   // engine allocation fails from here on
      for (int i = 0; i < MaxHeals; ++i) { simGlReset(b); heal(); }
      g_et.newArrFn = svArr; }
    KT_CHECK_EQ(slots()[0].heals, MaxHeals); KT_CHECK_EQ(countLines("tex emb_a: giving up after 20 refill attempts"), 1); KT_CHECK_EQ(countLines("last fill ok=0"), 1);
    KT_CHECK_EQ(countLines("art missing, labels still work"), 1);   // a real failure keeps the art-missing wording
    KT_CHECK_EQ(countLines("refilled after a texture reset"), 0);

    // ---- the 20th refill goes through the missing-rebuild branch: the line reports that branch's fill result ----
    for (int pass = 0; pass < 2; ++pass) {
        const bool fail = pass == 1;
        clearForTest(); resetCounters();
        KT_CHECK(ensure(set, 1)); b = B(slots()[0]);
        for (int i = 0; i < MaxHeals - 1; ++i) { simGlReset(b); heal(); }   // refills 1..19 via the texture-reset branch
        at<void*>(b, Tex_Pixels) = nullptr; at<int>(b, Tex_Uploaded) = 0; b[Tex_Constructed] = 1; b[Tex_Missing] = 1;
        auto svArr = g_et.newArrFn; if (fail) g_et.newArrFn = fNoMem;
        heal();   // refill 20: Reset + fill through the rebuild branch
        g_et.newArrFn = svArr;
        KT_CHECK_EQ(slots()[0].heals, MaxHeals); KT_CHECK_EQ(countLines("tex emb_a: rebuilt without our pixels"), 1);
        KT_CHECK_EQ(countLines("tex emb_a: refilled after a missing rebuild"), fail ? 0 : 1);
        KT_CHECK_EQ(countLines("tex emb_a: giving up after 20 refill attempts"), 1); KT_CHECK_EQ(countLines(fail ? "last fill ok=0" : "last fill ok=1"), 1);
        KT_CHECK_EQ(countLines("art missing"), fail ? 1 : 0);
    }

    // ---- give-up follows FAILED fills: the engine cannot allocate the pixel buffer, 20 retries then stop (even if the engine recovers) ----
    clearForTest(); resetCounters();
    { auto sv = g_et.newArrFn; g_et.newArrFn = fNoMem;
      KT_CHECK(!ensure(set, 1)); b = B(slots()[0]); KT_CHECK(b != nullptr); KT_CHECK_EQ(nNewArr, 1); KT_CHECK_EQ(nAdd, 0);   // registered, not filled
      for (int i = 0; i < 25; ++i) heal();
      KT_CHECK_EQ(nNewArr, 1 + MaxHeals); KT_CHECK_EQ(slots()[0].heals, MaxHeals); KT_CHECK_EQ(nAdd, 0);
      KT_CHECK_EQ(countLines("tex emb_a: engine malloc("), 1 + MaxHeals); KT_CHECK_EQ(countLines("tex emb_a: giving up after 20 refill attempts"), 1);
      g_et.newArrFn = sv; heal(); KT_CHECK_EQ(nAdd, 0); KT_CHECK_EQ(slots()[0].heals, MaxHeals); }   // given up stays given up until an upload re-arms it

    // ---- engine object allocation fails: the slot is registered without an object, ensure reports false, heal skips it ----
    clearForTest(); resetCounters();
    { auto sv = g_et.newFn; g_et.newFn = fNewFail;
      KT_CHECK(!ensure(set, 1)); KT_CHECK_EQ(slotCount(), 1); KT_CHECK(B(slots()[0]) == nullptr);
      KT_CHECK_EQ(nCtor, 0); KT_CHECK_EQ(nNewArr, 0); KT_CHECK_EQ(nAdd, 0); KT_CHECK_EQ(countLines("tex emb_a: allocation failed"), 1);
      heal(); KT_CHECK_EQ(nAdd, 0); KT_CHECK_EQ(nReset, 0);
      g_et.newFn = sv; KT_CHECK(!ensure(set, 1)); KT_CHECK_EQ(nCtor, 0); }   // the registered null slot is not retried by ensure (idempotent by name)

    // ---- bad TGAs rejected: nothing queued, a log line says why ----
    clearForTest();
    { unsigned char wrongType[sizeof kTga]; memcpy(wrongType, kTga, sizeof kTga); wrongType[2] = 10;   // RLE, not uncompressed
      unsigned char bpp24[sizeof kTga]; memcpy(bpp24, kTga, sizeof kTga); bpp24[16] = 24;
      unsigned char big[18 + 16]; memcpy(big, kTga, sizeof big); big[12] = 0x01; big[13] = 0x10;   // width 4097
      unsigned char trunc[sizeof kTga - 4]; memcpy(trunc, kTga, sizeof trunc);                       // one pixel short
      unsigned char huge[18]; memcpy(huge, kTga, 18); huge[12] = 0xFF; huge[13] = 0xFF; huge[14] = 0xFF; huge[15] = 0xFF;   // 65535 x 65535: no 32-bit wrap through
      unsigned char zero[18]; memcpy(zero, kTga, 18); zero[12] = 0; zero[13] = 0;
      TgaInfo ti;
      KT_CHECK_EQ((int)tgaInspect(wrongType, sizeof wrongType, &ti), (int)TGA_NOT_UNCOMPRESSED32); KT_CHECK_EQ((int)tgaInspect(bpp24, sizeof bpp24, &ti), (int)TGA_NOT_UNCOMPRESSED32);
      KT_CHECK_EQ((int)tgaInspect(big, sizeof big, &ti), (int)TGA_TOO_LARGE); KT_CHECK_EQ((int)tgaInspect(trunc, sizeof trunc, &ti), (int)TGA_TRUNCATED);
      KT_CHECK_EQ((int)tgaInspect(huge, sizeof huge, &ti), (int)TGA_TOO_LARGE); KT_CHECK_EQ((int)tgaInspect(zero, sizeof zero, &ti), (int)TGA_TRUNCATED);
      KT_CHECK_EQ((int)tgaInspect(kTga, 10, &ti), (int)TGA_NOT_UNCOMPRESSED32);
      resetCounters();
      const Tex bt[4] = {{"bad_type", wrongType, sizeof wrongType}, {"bad_big", big, sizeof big}, {"bad_trunc", trunc, sizeof trunc}, {"ok_c", kTga, sizeof kTga}};
      KT_CHECK(!ensure(bt, 4));   // the three bad ones fail, the good one is still created
      KT_CHECK_EQ(nCtor, 4); KT_CHECK_EQ(nNewArr, 1); KT_CHECK_EQ(nAdd, 1);
      KT_CHECK_EQ(countLines("tex bad_type: embedded TGA not uncompressed 32-bit"), 1); KT_CHECK_EQ(countLines("tex bad_big: embedded TGA too large (4097x2"), 1);
      KT_CHECK_EQ(countLines("tex bad_trunc: embedded TGA truncated"), 1); KT_CHECK_EQ(countLines("tex ok_c: created"), 1); }

    // ---- slot table limit and null args ----
    clearForTest(); resetCounters();
    { static char names[MaxTex + 1][16]; static Tex many[MaxTex + 1];
      for (int i = 0; i <= MaxTex; ++i) { snprintf(names[i], 16, "m%d", i); many[i] = Tex{names[i], kTga, sizeof kTga}; }
      KT_CHECK(!ensure(many, MaxTex + 1)); KT_CHECK_EQ(slotCount(), MaxTex); KT_CHECK_EQ(nCtor, MaxTex); KT_CHECK_EQ(countLines("slot table full"), 1); }
    KT_CHECK(!ensure(nullptr, 1)); KT_CHECK(!ensure(set, 0));

    // ---- non-Steam exe: no engine call at all ----
    clearForTest(); resetCounters();
    k2addr::setForTest(&kGog);
    KT_CHECK(!supported()); KT_CHECK(!ensure(set, 1)); KT_CHECK_EQ(nNew + nNewArr + nCtor + nAdd + nReset, 0); KT_CHECK_EQ(slotCount(), 0);
    k2addr::setForTest(nullptr); KT_CHECK(!ensure(set, 1)); KT_CHECK_EQ(nNew + nCtor, 0);   // unknown exe
    k2addr::clearForTest(); KT_CHECK(ensure(set, 1)); KT_CHECK_EQ(nCtor, 1); b = B(slots()[0]);
    k2addr::setForTest(&kGog); simGlReset(b); heal(); KT_CHECK_EQ(nAdd, 1); KT_CHECK_EQ(nReset, 0);   // heal is a no-op too
    k2addr::clearForTest();

    // ---- unready engine table / silent log ----
    { auto sv = g_et.ctorFn; g_et.ctorFn = nullptr; clearForTest(); resetCounters(); KT_CHECK(!ensure(set, 1)); KT_CHECK_EQ(nNew, 0); g_et.ctorFn = sv; }
    g_log = nullptr; clearForTest(); resetCounters(); KT_CHECK(ensure(set, 1)); KT_CHECK_EQ(g_nLines, 0);

    for (int i = 0; i < g_nAlloc; ++i) free(g_alloc[i]);
    return kt::Report("embtex_test");
}
