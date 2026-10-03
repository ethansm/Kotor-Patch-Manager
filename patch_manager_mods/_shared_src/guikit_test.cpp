// Host test of the GuiKit.h additions: imageFor out-params / 256 pool, createLabelFromTemplate failure reason.
// Fake engine in a MAP_32BIT arena (see research/companion_levelup/tools/parity/guikit_test.cpp for the full parity test).
#define GUIKIT_HOST 1
#include "GuiKit.h"
#include "check.h"
#include <sys/mman.h>
#include <stdio.h>

using namespace guikit;
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); } uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static int nLoad = 0, nAlloc = 0; static void* loadResult = reinterpret_cast<void*>(0x5000);
static bool allocFails = false; static uint32_t ctorVtable = LabelVtable; static bool initWritesId = true;
static void* fLoadImage(const char*) { ++nLoad; return loadResult; }
static void* fAlloc(unsigned n) { ++nAlloc; return allocFails ? nullptr : reinterpret_cast<void*>(arenaAlloc(n)); }
static void fCtor(void* t) { uintptr_t c = reinterpret_cast<uintptr_t>(t); at<uint32_t>(c, 0) = ctorVtable; at<int>(c, Ctl_Flags) = 0x2; }
static void* fExoCtor(void* e, const char* s) { memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fExoDtor(void*) {}
static void fInit(void*, void* ctl, void*, int, int) { if (initWritesId) at<int>(reinterpret_cast<uintptr_t>(ctl), Ctl_Id) = -1; }

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    void* vp = mmap(reinterpret_cast<void*>(0x00987000), 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (vp == MAP_FAILED) { perror("mmap 0x987000"); return 5; }
    Engine& e = g_gk;
    e.alloc = fAlloc; e.labelCtor = fCtor; e.initControl = fInit; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.loadImage = fLoadImage;

    // ---- imageFor (the pool is a function-local static: order matters) ----
    bool loaded = true; int pc = -1;
    KT_CHECK_EQ(reinterpret_cast<uintptr_t>(imageFor("", &loaded, &pc)), 0ull);
    KT_CHECK(!loaded); KT_CHECK_EQ(pc, 0);
    loaded = true; KT_CHECK(imageFor(nullptr, &loaded, &pc) == nullptr && !loaded && pc == 0);
    void* a = imageFor("FPH_Slot", &loaded, &pc);
    KT_CHECK(loaded); KT_CHECK_EQ(pc, 1);
    KT_CHECK(a == loadResult); KT_CHECK_EQ(nLoad, 1);
    void* b = imageFor("FPH_Slot", &loaded, &pc);
    KT_CHECK(!loaded); KT_CHECK_EQ(pc, 1);
    KT_CHECK(b == a);
    void* c = imageFor("fph_slot", &loaded, &pc);
    KT_CHECK(!loaded && pc == 1 && c == a); KT_CHECK_EQ(nLoad, 1);
    KT_CHECK(imageFor("fph_slot") == a);
    // null result is cached
    loadResult = nullptr;
    KT_CHECK(imageFor("nullimg", &loaded, &pc) == nullptr && loaded && pc == 2);
    KT_CHECK(imageFor("nullimg", &loaded, &pc) == nullptr && !loaded && pc == 2);
    KT_CHECK_EQ(nLoad, 2);
    loadResult = reinterpret_cast<void*>(0x5000);
    KT_CHECK(imageFor("plain_new") == loadResult);   // pool = 3
    // fill to ImagePoolSize
    char nm[24]; bool allOk = true;
    for (int i = 3; i < ImagePoolSize; ++i) { snprintf(nm, sizeof nm, "fill%d", i); allOk &= imageFor(nm, &loaded, &pc) != nullptr && loaded && pc == i + 1; }
    KT_CHECK(allOk); KT_CHECK_EQ(ImagePoolSize, 256);
    int before = nLoad;
    void* full = imageFor("one_too_many", &loaded, &pc);
    KT_CHECK(full == nullptr); KT_CHECK(!loaded);
    KT_CHECK_EQ(pc, ImagePoolSize); KT_CHECK_EQ(nLoad, before);
    KT_CHECK(imageFor("fph_slot", &loaded, &pc) == a && !loaded && pc == ImagePoolSize);

    // ---- createLabelFromTemplate ----
    uintptr_t panel = arenaAlloc(0x100); at<int>(panel, Panel_CtlCount) = 7;
    int why = -1; uint32_t bad = 123;
    uintptr_t lbl = createLabelFromTemplate(panel, "LBL_X", &why, &bad);
    KT_CHECK(lbl != 0); KT_CHECK_EQ(why, (int)CF_None);
    KT_CHECK(lbl && (at<int>(lbl, Ctl_Flags) & FlagVisible) == 0 && (at<int>(lbl, Ctl_Flags) & FlagClickThrough));
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X") != 0);

    allocFails = true; why = -1;
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Alloc);
    allocFails = false;
    why = -1; KT_CHECK(createLabelFromTemplate(0, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Alloc);

    ctorVtable = 0xABCD0000u; why = -1; bad = 0;
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why, &bad) == 0);
    KT_CHECK_EQ(why, (int)CF_Vtable); KT_CHECK_EQ((unsigned long long)bad, 0xABCD0000ull);
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0 && why == CF_Vtable);
    ctorVtable = LabelVtable;

    initWritesId = false; why = -1;
    KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Template);
    initWritesId = true;

    why = -1;   // panel whose count field (0x38) sits in the unreadable low page
    KT_CHECK(createLabelFromTemplate(0x10, "LBL_X", &why) == 0); KT_CHECK_EQ(why, (int)CF_Count);

    why = -1; KT_CHECK(createLabelFromTemplate(panel, "LBL_X", &why) != 0 && why == CF_None);
    return kt::Report("guikit_test");
}
