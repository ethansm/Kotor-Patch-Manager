// Host test of EmbedGui.h: open an in-memory "GUI " layout on a fake panel, close it, and the Malloc-fail, parse-fail, null-panel and
// double-close paths. Fake engine = the seven EMBEDGUI_HOST wrappers defined below (host heap buffers plus an event trace, one char per
// engine call: N new, C ctor, M Malloc, P parse, F FreeResourceData, R release 0x7112B0, D vt[0](1)). Expected values are literals.
#ifndef EMBEDGUI_HOST
#define EMBEDGUI_HOST 1
#endif
#include "EmbedGui.h"
#include "check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace embedgui;

// ---- fake engine state ----
static char g_ev[32]; static int g_nEv = 0;
static void ev(char c) { if (g_nEv < 31) { g_ev[g_nEv++] = c; g_ev[g_nEv] = 0; } }
static const char* evStr() { return g_ev; }
static int nFreeData = 0, nRelease = 0, nDtor = 0, nLog = 0;
static unsigned g_newSize = 0;
static bool g_mallocOk = true;
static void* g_freedRes = nullptr;          // FreeResourceData target
static void* g_releasedObj = nullptr;       // 0x7112B0 target
static void* g_slotAtRelease = nullptr;     // P+0x30 at the moment of the release call
static void* g_dtorObj = nullptr;
static unsigned char g_panel[0x80];         // fake panel: only +0x30 is used
static unsigned char g_panel2[0x80];
static char g_vanilla[4];                   // stand-in for the vanilla layout object
static char g_other[4];                     // stand-in for a layout someone else swapped in
static void* g_track[256]; static int g_nTrack = 0;   // host heap, freed at exit (LeakSanitizer); the real engine owns these
static void* track(void* p) { if (g_nTrack < 256) g_track[g_nTrack++] = p; return p; }
static void resetCounters() { g_nEv = 0; g_ev[0] = 0; nFreeData = nRelease = nDtor = nLog = 0; g_mallocOk = true; }
static void resetPanel(unsigned char* P) { layoutSlot(P) = g_vanilla; }
static void fLog(const char* fmt, ...) { (void)fmt; ++nLog; }

// The seven wrappers (EMBEDGUI_HOST). The data-buffer copy and the parse see real bytes; the vtable is never read on the host.
namespace embedgui {
void* callNew(unsigned n) { ev('N'); g_newSize = n; return track(calloc(1, n)); }
void callCtor(void* obj) {   // the real ctor also sets ID -1 at +8 (no owner, no data)
    ev('C');
    *reinterpret_cast<int*>(static_cast<unsigned char*>(obj) + 0x8) = -1;
}
bool callMalloc(void* obj) {
    ev('M');
    if (!g_mallocOk) return false;
    unsigned char* o = static_cast<unsigned char*>(obj);
    unsigned n = *reinterpret_cast<uint32_t*>(o + 0x18);
    *reinterpret_cast<void**>(o + 0x10) = track(calloc(1, n));
    return true;
}
int callParse(void* obj) {   // OnResourceServiced: signature "GUI " then version "V3.2", else fail
    ev('P');
    const unsigned char* d = *reinterpret_cast<unsigned char* const*>(static_cast<unsigned char*>(obj) + 0x10);
    return (d && memcmp(d, "GUI V3.2", 8) == 0) ? 1 : 0;
}
void callFreeData(void* obj) { ev('F'); ++nFreeData; g_freedRes = obj; }
void callRelease(void* obj) { ev('R'); ++nRelease; g_releasedObj = obj; g_slotAtRelease = layoutSlot(g_panel); }
void callDtor(void* obj) { ev('D'); ++nDtor; g_dtorObj = obj; }
}  // namespace embedgui

// 16-byte GFF: signature "GUI ", version "V3.2", then 8 payload bytes. kBad has the wrong version, so the parse fails.
static const unsigned char kGff[16] = {'G', 'U', 'I', ' ', 'V', '3', '.', '2', 1, 2, 3, 4, 5, 6, 7, 8};
static const unsigned char kBad[16] = {'G', 'U', 'I', ' ', 'V', '9', '.', '9', 1, 2, 3, 4, 5, 6, 7, 8};

int main() {
    g_log = fLog;

    // ---- constants (Steam decomp, D2F section 1) ----
    KT_CHECK_EQ(ObjBytes, 0xA0);
    KT_CHECK_EQ(GffType, 0x7FFu);
    KT_CHECK_EQ(GffExt, "GUI ");
    KT_CHECK_EQ(DummyResref, "mod_inline_gui");
    KT_CHECK_EQ(Obj_Data, 0x10);
    KT_CHECK_EQ(Obj_DataSize, 0x18);
    KT_CHECK_EQ(PanelLayoutSlot, 0x30);

    // (c) null panel: null handle, no engine call; Close of it is a no-op
    resetCounters();
    Handle n = Open(nullptr, kGff, sizeof kGff);
    KT_CHECK(n.obj == nullptr);
    KT_CHECK_EQ(g_nEv, 0);
    Close(nullptr, n);
    KT_CHECK_EQ(g_nEv, 0);

    // (a)(b) open swaps P+0x30 to obj; close restores it; success flags, buffer and sizes
    resetCounters(); resetPanel(g_panel);
    Handle h = Open(g_panel, kGff, sizeof kGff);
    KT_CHECK(h.obj != nullptr);
    KT_CHECK(h.saved == static_cast<void*>(g_vanilla));
    KT_CHECK(layoutSlot(g_panel) == h.obj);                  // (b) swap window
    KT_CHECK_EQ(evStr(), "NCMP");
    KT_CHECK_EQ(g_newSize, 0xA0u);
    {
        unsigned char* o = static_cast<unsigned char*>(h.obj);
        KT_CHECK_EQ(*reinterpret_cast<int*>(o + 0x8), -1);                 // ID stays none
        KT_CHECK_EQ(*reinterpret_cast<uint32_t*>(o + 0xC) & 0x4u, 0x4u);   // has-data flag
        KT_CHECK_EQ(*reinterpret_cast<uint16_t*>(o + 0x4), 1);             // refcount 1
        KT_CHECK_EQ(*reinterpret_cast<uint32_t*>(o + 0x9C), 1u);
        KT_CHECK_EQ(*reinterpret_cast<uint32_t*>(o + 0x18), 16u);          // size
        KT_CHECK_EQ(*reinterpret_cast<uint32_t*>(o + 0x1C), 0u);           // attach count untouched
        KT_CHECK(memcmp(*reinterpret_cast<unsigned char**>(o + 0x10), kGff, sizeof kGff) == 0);   // bytes in the buffer
    }
    void* obj = h.obj;
    Close(g_panel, h);
    KT_CHECK_EQ(evStr(), "NCMPRD");                                        // release (0x7112B0) then vt[0](1)
    KT_CHECK(g_slotAtRelease == static_cast<void*>(g_vanilla));            // (a) restored before the release
    KT_CHECK(layoutSlot(g_panel) == static_cast<void*>(g_vanilla));        // (a) P+0x30 == saved
    KT_CHECK(g_releasedObj == obj);
    KT_CHECK(g_dtorObj == obj);
    KT_CHECK(h.obj == nullptr);
    KT_CHECK_EQ(nFreeData, 0);
    KT_CHECK_EQ(nLog, 5);                                                  // new, ctor, Malloc, parsed, close

    // (f) second Close is a no-op: nothing freed, P+0x30 left as the caller set it
    void* other = g_other;
    layoutSlot(g_panel) = other;
    Close(g_panel, h);
    KT_CHECK_EQ(evStr(), "NCMPRD");
    KT_CHECK_EQ(nRelease, 1);
    KT_CHECK_EQ(nDtor, 1);
    KT_CHECK(layoutSlot(g_panel) == other);

    // (d) Malloc false: null, no swap, no free
    resetCounters(); resetPanel(g_panel); g_mallocOk = false;
    Handle m = Open(g_panel, kGff, sizeof kGff);
    KT_CHECK(m.obj == nullptr);
    KT_CHECK(layoutSlot(g_panel) == static_cast<void*>(g_vanilla));
    KT_CHECK_EQ(evStr(), "NCM");
    KT_CHECK_EQ(nFreeData, 0);
    KT_CHECK_EQ(nRelease, 0);
    KT_CHECK_EQ(nDtor, 0);
    g_mallocOk = true;

    // (e) parse fail: FreeResourceData (0x7120C0) then vt[0](1) once; P+0x30 never swapped
    resetCounters(); resetPanel(g_panel);
    Handle p = Open(g_panel, kBad, sizeof kBad);
    KT_CHECK(p.obj == nullptr);
    KT_CHECK(layoutSlot(g_panel) == static_cast<void*>(g_vanilla));
    KT_CHECK_EQ(evStr(), "NCMPFD");
    KT_CHECK_EQ(nFreeData, 1);
    KT_CHECK_EQ(nDtor, 1);
    KT_CHECK_EQ(nRelease, 0);
    KT_CHECK(g_freedRes != nullptr && g_freedRes == g_dtorObj);

    // (g) one free per successful handle: two opens on two panels, two closes
    resetCounters(); resetPanel(g_panel); resetPanel(g_panel2);
    Handle a = Open(g_panel, kGff, sizeof kGff);
    Handle b = Open(g_panel2, kGff, sizeof kGff);
    KT_CHECK(a.obj != nullptr && b.obj != nullptr && a.obj != b.obj);
    Close(g_panel, a);
    Close(g_panel2, b);
    KT_CHECK_EQ(evStr(), "NCMPNCMPRDRD");
    KT_CHECK_EQ(nRelease, 2);
    KT_CHECK_EQ(nDtor, 2);
    KT_CHECK_EQ(nFreeData, 0);
    KT_CHECK(layoutSlot(g_panel) == static_cast<void*>(g_vanilla));
    KT_CHECK(layoutSlot(g_panel2) == static_cast<void*>(g_vanilla));

    for (int i = 0; i < g_nTrack; ++i) free(g_track[i]);
    return kt::Report("embedgui_test");
}
