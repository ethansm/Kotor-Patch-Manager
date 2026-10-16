// Host test of GuiButton.h: createButtonFromTemplate (ok path, wrong vtable, template missing, canary overwrite, alloc failure,
// Steam gate, null args), setButtonText vtable gate, buttonHighlighted bit, releaseUnlinkedButton rule.
// Fake engine: objects live in a static arena; the page holding the CSWGuiButton vftable (0x00987A1C) is mapped so the release
// path can read vtable slot 0 (the seam records the call instead of making it). Expected values are independent literals.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "GuiButton.h"
#include "check.h"
#include <sys/mman.h>
#include <stdio.h>

using namespace guikit;
using namespace guibutton;

static uint8_t g_arena[1u << 16] __attribute__((aligned(16)));
static size_t g_used = 0;

static int nAlloc = 0, nCtor = 0, nInit = 0, nExoCtor = 0, nTextSet = 0;
static bool allocFails = false;
static bool templateFound = true;          // false: the fake InitControl leaves the id at IdSentinel (tag not in the layout)
static uint32_t ctorVt = ButtonVtable;     // what the fake ctor writes at +0
static int ctorOverrunAt = -1;             // >= 0: the fake ctor also writes 4 bytes at ButtonSize + this offset (canary test)
static uintptr_t lastCtorSelf = 0, initPanel = 0, initCtl = 0;
static int initAddToList = -1, initScale = -1;
static char lastExoText[16] = {0};
static uintptr_t textSetEcx = 0;
static char textSetArg[16] = {0};
static int dtorCalls = 0; static uint32_t dtorFn = 0; static uintptr_t dtorSelf = 0; static unsigned dtorFlags = 0;

constexpr uint32_t FakeDtorFn = 0x00ABCDE0u;   // stored in slot 0 of the mapped ButtonVtable page; never called
static uint32_t* const SlotZeroAddr = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(ButtonVtable));

template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static void copyTo(char* dst, size_t cap, const char* src) {
    size_t i = 0;
    for (; i + 1 < cap && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}

static uintptr_t arenaBlock(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > sizeof g_arena) { printf("arena full\n"); exit(4); }
    uintptr_t p = reinterpret_cast<uintptr_t>(g_arena + g_used);
    memset(reinterpret_cast<void*>(p), 0xEE, n);   // junk, so only the header's own memset/init can make the object clean
    g_used += n;
    return p;
}

static uintptr_t newPanel() {
    uintptr_t p = arenaBlock(0x100);
    at<int>(p, Panel_CtlCount) = 3;
    return p;
}

// ---- fake engine seams ----
static void* fAlloc(unsigned n) {
    ++nAlloc;
    if (allocFails) return nullptr;
    return reinterpret_cast<void*>(arenaBlock(n));
}
static void fCtor(void* self) {
    ++nCtor;
    uintptr_t c = reinterpret_cast<uintptr_t>(self);
    lastCtorSelf = c;
    at<uint32_t>(c, 0) = ctorVt;
    at<int>(c, Ctl_Flags) = FlagVisible;   // the engine creates controls visible; the header hides them
    if (ctorOverrunAt >= 0) at<uint32_t>(c, ButtonSize + ctorOverrunAt) = 0x12345678u;
}
static void* fExoCtor(void* e, const char* s) { ++nExoCtor; copyTo(static_cast<char*>(e), 16, s); copyTo(lastExoText, 16, s); return e; }
static void fExoDtor(void*) {}
static void fInit(void* panel, void* ctl, void*, int addToList, int scale) {
    ++nInit;
    initPanel = reinterpret_cast<uintptr_t>(panel);
    initCtl = reinterpret_cast<uintptr_t>(ctl);
    initAddToList = addToList;
    initScale = scale;
    if (templateFound) at<int>(reinterpret_cast<uintptr_t>(ctl), Ctl_Id) = 7;
}
static void fTextSet(void* self, void* str) {
    ++nTextSet;
    textSetEcx = reinterpret_cast<uintptr_t>(self);
    copyTo(textSetArg, 16, static_cast<const char*>(str));
}
// slot-0 seam: the header hands (slot value, object, flags) here instead of calling into the page
static void recordDtor(uint32_t fn, uintptr_t self, unsigned flags) { ++dtorCalls; dtorFn = fn; dtorSelf = self; dtorFlags = flags; }

static void resetFake() {
    nAlloc = nCtor = nInit = nExoCtor = nTextSet = 0;
    allocFails = false; templateFound = true; ctorVt = ButtonVtable; ctorOverrunAt = -1;
    lastCtorSelf = initPanel = initCtl = 0; initAddToList = initScale = -1;
    lastExoText[0] = 0; textSetEcx = 0; textSetArg[0] = 0;
    dtorCalls = 0; dtorFn = 0; dtorSelf = 0; dtorFlags = 0;
    k2addr::clearForTest();
}

int main() {
    void* vp = mmap(reinterpret_cast<void*>(0x00987000), 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (vp == MAP_FAILED) { perror("mmap 0x987000"); return 5; }
    *SlotZeroAddr = FakeDtorFn;
    g_stubVirtualQueryAll = true;
    g_gk.alloc = fAlloc; g_gk.initControl = fInit; g_gk.exoCtor = fExoCtor; g_gk.exoDtor = fExoDtor; g_gk.textSet = fTextSet;
    g_btnCtor = fCtor;
    g_gkCallLabelDtor = recordDtor;

    // ---- ok path ----
    resetFake();
    uintptr_t panel = newPanel();
    int why = -1; uint32_t bad = 0xFFFFu; bool canary = false;
    uintptr_t btn = createButtonFromTemplate(panel, "BTN_OK", &why, &bad, &canary);
    KT_CHECK(btn != 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_None));
    KT_CHECK_EQ(static_cast<unsigned long long>(bad), 0ull);
    KT_CHECK(canary);
    KT_CHECK_EQ(static_cast<unsigned long long>(at<uint32_t>(btn, 0)), static_cast<unsigned long long>(ButtonVtable));
    KT_CHECK_EQ(at<int>(btn, Ctl_Flags), FlagClickThrough);   // hidden (bit 1 clear) + click-through
    KT_CHECK_EQ(at<int>(btn, Ctl_Id), 3);                       // provisional id = panel count
    KT_CHECK_EQ(nCtor, 1);
    KT_CHECK_EQ(nInit, 1);
    KT_CHECK(initPanel == panel);
    KT_CHECK(initCtl == btn);
    KT_CHECK_EQ(initAddToList, 0);
    KT_CHECK_EQ(initScale, 1);
    KT_CHECK(strcmp(lastExoText, "BTN_OK") == 0);
    KT_CHECK_EQ(dtorCalls, 0);
    KT_CHECK_EQ(nAlloc, 1);

    // ---- wrong vtable: CF_Vtable, badVt reported, object leaked (never released) ----
    resetFake(); ctorVt = LabelVtable;
    panel = newPanel();
    btn = createButtonFromTemplate(panel, "BTN_OK", &why, &bad, &canary);
    KT_CHECK(btn == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Vtable));
    KT_CHECK_EQ(static_cast<unsigned long long>(bad), static_cast<unsigned long long>(LabelVtable));
    KT_CHECK(!canary);
    KT_CHECK_EQ(dtorCalls, 0);
    KT_CHECK_EQ(nInit, 0);

    // ---- template tag not in the layout: CF_Template, released exactly once through slot 0 ----
    resetFake(); templateFound = false;
    panel = newPanel();
    btn = createButtonFromTemplate(panel, "BTN_NOPE", &why, &bad, &canary);
    KT_CHECK(btn == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Template));
    KT_CHECK(!canary);
    KT_CHECK_EQ(nInit, 1);
    KT_CHECK_EQ(dtorCalls, 1);
    KT_CHECK_EQ(static_cast<unsigned long long>(dtorFn), static_cast<unsigned long long>(FakeDtorFn));
    KT_CHECK(dtorSelf == lastCtorSelf);
    KT_CHECK_EQ(dtorFlags, 1u);

    // ---- ctor overruns into the canary: object still returned, canaryOk false ----
    resetFake(); ctorOverrunAt = 4;
    panel = newPanel();
    btn = createButtonFromTemplate(panel, "BTN_OK", &why, &bad, &canary);
    KT_CHECK(btn != 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_None));
    KT_CHECK(!canary);
    KT_CHECK_EQ(dtorCalls, 0);

    // ---- allocation fails: CF_Alloc, ctor never runs ----
    resetFake(); allocFails = true;
    panel = newPanel();
    btn = createButtonFromTemplate(panel, "BTN_OK", &why, &bad, &canary);
    KT_CHECK(btn == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Alloc));
    KT_CHECK_EQ(nCtor, 0);

    // ---- Steam gate: null table and the GOG table both refuse before any allocation ----
    resetFake();
    panel = newPanel();
    k2addr::setForTest(nullptr);
    btn = createButtonFromTemplate(panel, "BTN_OK", &why, &bad, &canary);
    KT_CHECK(btn == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Alloc));
    KT_CHECK_EQ(nAlloc, 0);
    KT_CHECK(!steamBuild());
    k2addr::setForTest(&kGog);
    btn = createButtonFromTemplate(panel, "BTN_OK", &why, &bad, &canary);
    KT_CHECK(btn == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Alloc));
    KT_CHECK_EQ(nAlloc, 0);
    k2addr::clearForTest();
    KT_CHECK(steamBuild());

    // ---- null arguments and missing seam: failure, no calls ----
    resetFake();
    panel = newPanel();
    KT_CHECK(createButtonFromTemplate(0, "BTN_OK", &why) == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Alloc));
    KT_CHECK(createButtonFromTemplate(panel, nullptr, &why) == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Alloc));
    KT_CHECK_EQ(nAlloc, 0);
    KT_CHECK(createButtonFromTemplate(panel, "BTN_OK") != 0);   // out-params are optional
    resetFake();
    CtorFn savedCtor = g_btnCtor;
    g_btnCtor = nullptr;
    panel = newPanel();
    KT_CHECK(createButtonFromTemplate(panel, "BTN_OK", &why) == 0);
    KT_CHECK_EQ(why, static_cast<int>(CF_Alloc));
    KT_CHECK_EQ(nAlloc, 0);
    g_btnCtor = savedCtor;

    // ---- setButtonText: vtable gate on btn+0x160, TextSet ECX = btn+0x160+0x18 ----
    resetFake();
    uintptr_t b = arenaBlock(ButtonSize + CanaryBytes);
    at<uint32_t>(b, Button_Text) = TextVtable;
    KT_CHECK(setButtonText(b, "Continue"));
    KT_CHECK_EQ(nTextSet, 1);
    KT_CHECK(textSetEcx == b + static_cast<uintptr_t>(Button_Text + Text_StringSub));
    KT_CHECK(strcmp(textSetArg, "Continue") == 0);
    at<uint32_t>(b, Button_Text) = 0x1234u;                     // mismatch: refused, no TextSet
    KT_CHECK(!setButtonText(b, "Nope"));
    KT_CHECK_EQ(nTextSet, 1);
    at<uint32_t>(b, Button_Text) = TextVtable;
    k2addr::setForTest(&kGog);                                  // non-Steam table: refused
    KT_CHECK(!setButtonText(b, "Gog"));
    k2addr::clearForTest();
    KT_CHECK_EQ(nTextSet, 1);
    KT_CHECK(!setButtonText(0, "x"));                           // null button
    KT_CHECK(!setButtonText(b, nullptr));                       // null text
    KT_CHECK_EQ(nTextSet, 1);

    // ---- buttonHighlighted: ctl+0x48 bit 0 ----
    at<int>(b, Ctl_Flags) = 0x1;
    KT_CHECK(buttonHighlighted(b));
    at<int>(b, Ctl_Flags) = 0x2;                                // visible only
    KT_CHECK(!buttonHighlighted(b));
    at<int>(b, Ctl_Flags) = 0x21;                               // highlight + click-through
    KT_CHECK(buttonHighlighted(b));
    KT_CHECK(!buttonHighlighted(0));

    // ---- releaseUnlinkedButton: only ButtonVtable with a non-zero slot 0 is called, with flags = 1 ----
    resetFake();
    uintptr_t bb = arenaBlock(ButtonSize + CanaryBytes);
    at<uint32_t>(bb, 0) = ButtonVtable;
    releaseUnlinkedButton(bb);
    KT_CHECK_EQ(dtorCalls, 1);
    KT_CHECK_EQ(static_cast<unsigned long long>(dtorFn), static_cast<unsigned long long>(FakeDtorFn));
    KT_CHECK(dtorSelf == bb);
    KT_CHECK_EQ(dtorFlags, 1u);
    at<uint32_t>(bb, 0) = LabelVtable;                          // wrong vtable: not called
    releaseUnlinkedButton(bb);
    KT_CHECK_EQ(dtorCalls, 1);
    at<uint32_t>(bb, 0) = ButtonVtable;
    *SlotZeroAddr = 0;                                          // zero slot 0: not called
    releaseUnlinkedButton(bb);
    KT_CHECK_EQ(dtorCalls, 1);
    *SlotZeroAddr = FakeDtorFn;
    k2addr::setForTest(nullptr);                                // non-Steam: not called
    releaseUnlinkedButton(bb);
    KT_CHECK_EQ(dtorCalls, 1);
    k2addr::clearForTest();
    releaseUnlinkedButton(0);
    KT_CHECK_EQ(dtorCalls, 1);

    return kt::Report("gui_button_test");
}
