// common_contract_test.cpp -- Patches/Common CONTRACT test for the Character Effects Panel (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/{saferead,windowhook}_test.cpp). Every message starts with
// "chareffects relies on: " so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY
// edit to SafeRead.h / WindowHook.h / GameAddr.h re-runs this test: a Common change that would break the panel fails here first.
//
// Areas (also EffectIcons.h / Effects.h: icon tables, item-ability set, world-time chain; section 2b; GuiKit.h: setRect / getExtent / imageFor / textObjOk / setText / setTextColor / setFill, section 2c): (1) gameaddr constants (compile time), (2) saferead via the mod's readAt wrapper call shape (site 0), (3) WindowHook<WheelTag>
// lifecycle with a proc that mirrors the mod's wheelProc (handle when active, else forward). Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "check.h"
#include "fake_wndproc.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "K2Addr.h"
#include "EffectIcons.h"
#include "GuiKit.h"
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using HWND = fakewnd::HWND;
using WNDPROC = fakewnd::Proc;
using LRESULT = intptr_t;
using UINT = unsigned;
using WPARAM = intptr_t;
using LPARAM = intptr_t;

namespace kp { namespace whapi {
inline int pinCalls = 0, setCalls = 0, defHits = 0;
constexpr LRESULT kDef = 0x5EED;
inline WNDPROC setProc(HWND h, WNDPROC p) { ++setCalls; return fakewnd::FakeSetWindowLongPtr(h, p); }
inline WNDPROC getProc(HWND h) { return fakewnd::FakeGetWindowLongPtr(h); }
inline LRESULT callProc(WNDPROC p, HWND h, UINT m, WPARAM w, LPARAM l) { return fakewnd::FakeCallWindowProc(p, h, m, w, l); }
inline LRESULT defProc(HWND, UINT, WPARAM, LPARAM) { ++defHits; return kDef; }
inline bool isWindow(HWND h) { return fakewnd::FakeIsWindow(h); }
inline void pin(WNDPROC) { ++pinCalls; }
inline LONG xchg(volatile LONG* t, LONG v) { return __atomic_exchange_n(t, v, __ATOMIC_SEQ_CST); }
} }
#include "WindowHook.h"

#define HCHECK(why, expr) ::kt::Check("chareffects relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("chareffects relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))
#define U(x) static_cast<unsigned long long>(x)

// ============================================================ 1. GameAddr (compile time)
#define HADDR(name, val) static_assert(gameaddr::name == (val), "chareffects relies on: gameaddr::" #name " == " #val " (validated Steam address)")
HADDR(ClientCreatureVtable, 0x0099EE14); HADDR(WorldTimerGetCurrentTime, 0x0051AD20);

// ============================================================ 2. SafeRead (the mod's readAt wrapper shape, site 0)
// The mod's wrapper truncates the base to 32 bits (engine pointers); on the 64-bit host the buffer therefore lives in a MAP_32BIT arena.
static uint8_t* g_arena = nullptr;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
template <typename T> static bool modRead(const void* base, int off, T* out) {   // exact call shape of CharEffectsPanel readAt<T>
    return saferead::readAt<T>(0, static_cast<uintptr_t>(static_cast<unsigned>(reinterpret_cast<uintptr_t>(base))), off, out);
}
static void saferead_contract() {
    memset(g_arena, 0, 64);
    at<int>(reinterpret_cast<uintptr_t>(g_arena), 8) = 1920;
    at<short>(reinterpret_cast<uintptr_t>(g_arena), 12) = -7;
    at<float>(reinterpret_cast<uintptr_t>(g_arena), 16) = 0.5f;
    saferead::beginScope();
    int i = 0x11111111; short s = 99; float f = 1.f;
    HCHECK("readAt<int> on readable memory returns true (framebuffer size, effect counts)", modRead(g_arena, 8, &i));
    HEQ("readAt<int> delivers the value", (long long)i, 1920ll);
    HCHECK("readAt<short> works (effect fields)", modRead(g_arena, 12, &s) && s == -7);
    HCHECK("readAt<float> works (durations)", modRead(g_arena, 16, &f) && f == 0.5f);
    i = 0x22222222;
    HCHECK("readAt with base 0 returns false", !modRead(nullptr, 8, &i));
    HEQ("readAt failure leaves *out untouched (base 0; the mod pre-seeds defaults)", (long long)i, 0x22222222ll);
    HCHECK("readAt below 0x10000 returns false", !modRead(reinterpret_cast<void*>(0x20), 0, &i));
    HEQ("readAt failure leaves *out untouched (low address)", (long long)i, 0x22222222ll);
    s = 55; f = 2.f;
    HCHECK("readAt<short>/<float> failures return false", !modRead(reinterpret_cast<void*>(0x30), 4, &s) && !modRead(nullptr, 0, &f));
    HCHECK("readAt<short>/<float> failures leave *out untouched", s == 55 && f == 2.f);
    bool ok = true;
    for (int n = 0; n < 5; ++n) { saferead::beginScope(); i = 0; ok = ok && modRead(g_arena, 8, &i) && i == 1920; }
    HCHECK("beginScope is callable repeatedly (every poll) and reads still work", ok);
}

// ============================================================ 2b. EffectIcons / k2fx (icon tables, item-ability set, world time chain)
static int g_fakeDay = 0;
static void fakeGetTime(uintptr_t timer, int* day, int* ms) { g_fakeDay = static_cast<int>(timer); *day = 7; *ms = 1234; }
static void effecticons_contract() {
    bool item = true;
    const char* ic = k2fx::iconForSpell(8, false, &item);
    HCHECK("k2fx::iconForSpell(8) buff row is tracked (ip_speedburst), not on the item disc", ic && !strcmp(ic, "ip_speedburst") && !item);
    HCHECK("k2fx::iconForSpell(257) is an item-ability buff on the disc (shield)", k2fx::iconForSpell(257, false, &item) && item);
    const char* bad = k2fx::iconForSpell(4, true, &item);
    HCHECK("k2fx::iconForSpell(4, debuff) is tracked (ip_advlsthrow)", bad && !strcmp(bad, "ip_advlsthrow") && !item);
    HCHECK("k2fx::iconForSpell out of range (-1, 287) is nullptr", !k2fx::iconForSpell(-1, false, &item) && !k2fx::iconForSpell(287, true, &item));
    HEQ("icon tables cover 287 spells.2da rows (the panel guards with kEffectIconTableSize)", kEffectIconTableSize, 287);
    HEQ("debuff table is the same size as the buff table", kDebuffIconTableSize, kEffectIconTableSize);
    HCHECK("kDebuffIcons[4] resref/item readable (the panel reads the table directly)", kDebuffIcons[4].resref && !kDebuffIcons[4].item);
    HCHECK("k2fx::isItemAbility(69) true (stim range start)", k2fx::isItemAbility(69));
    HCHECK("k2fx::isItemAbility 77/99/108/110/115/129/132/257 true", k2fx::isItemAbility(77) && k2fx::isItemAbility(99) && k2fx::isItemAbility(108) &&
           k2fx::isItemAbility(110) && k2fx::isItemAbility(115) && k2fx::isItemAbility(129) && k2fx::isItemAbility(132) && k2fx::isItemAbility(257));
    HCHECK("k2fx::isItemAbility false for force-power rows 8, 24 and range gaps 68, 78, 109, 256", !k2fx::isItemAbility(8) && !k2fx::isItemAbility(24) &&
           !k2fx::isItemAbility(68) && !k2fx::isItemAbility(78) && !k2fx::isItemAbility(109) && !k2fx::isItemAbility(256));

    // world time: AppGlobal -> +8 -> +4 -> +0x10048 timer (Steam offsets the panel's old local currentTime used)
    uintptr_t base = reinterpret_cast<uintptr_t>(g_arena);
    memset(g_arena, 0, 0x10400);
    uintptr_t ag = base, w = base + 0x100, sub = base + 0x200, w2 = base + 0x300;
    at<int>(ag, 0) = static_cast<int>(w);               // *(AppGlobal)
    at<int>(w, 8) = static_cast<int>(w2);               // +8 -> world
    at<int>(w2, 4) = static_cast<int>(sub);             // +4 -> sub
    k2fx::Engine saved = k2fx::g_fx;
    k2fx::g_fx.appGlobal = ag; k2fx::g_fx.getTime = &fakeGetTime;
    int day = -1, ms = -1;
    saferead::beginScope();
    HCHECK("k2fx::currentTime false while the world timer is null (main menu / load)", !k2fx::currentTime(&day, &ms));
    at<int>(sub, 0x10048) = 0x5150;
    saferead::beginScope();
    HCHECK("k2fx::currentTime true once the timer exists; day/ms from the engine call", k2fx::currentTime(&day, &ms) && day == 7 && ms == 1234);
    HEQ("k2fx::currentTime passes the timer read at sub+0x10048 to the engine call", g_fakeDay, 0x5150);
    k2fx::g_fx = saved;
}

// ============================================================ 2c. GuiKit (the helpers the panel's ctlRect / setCtlRect / imageFor / textObjOk / setText / setTextColor / setFill delegate to)
// Steam table == the constants the panel used to hardcode (FnLoadImage 0x47EB60, FnTextSet 0x416E30, FnTextColor 0x417140, FnExoCtor/Dtor, vtables, offsets).
static_assert(kSteam.GuiLoadImage == 0x0047EB60 && kSteam.TextSet == 0x00416E30 && kSteam.TextColor == 0x00417140, "chareffects relies on: Steam image/text engine calls");
static_assert(kSteam.ExoCtor == 0x00733570 && kSteam.ExoDtor == 0x00733780, "chareffects relies on: Steam CExoString ctor/dtor");
static_assert(kSteam.BorderVtable == 0x009875BC && kSteam.TextVtable == 0x009876B4, "chareffects relies on: Steam border/text vtables");
static_assert(kSteam.Ctl_X == 4 && kSteam.Ctl_H == 0x10 && kSteam.Ctl_VtSetRect == 4, "chareffects relies on: Steam extent offsets and SetRect vtable slot 1");
static_assert(kSteam.Label_Border == 0x60 && kSteam.Border_Tint == 0x28 && kSteam.Border_Fill == 0x74, "chareffects relies on: Steam border offsets");
static_assert(kSteam.Label_Text == 0xD8 && kSteam.Text_Renderer == 0x14 && kSteam.Text_StringSub == 0x18, "chareffects relies on: Steam label text offsets");

static uintptr_t g_gkNext = 0x12000;
static uintptr_t gkAlloc(int n) { uintptr_t p = reinterpret_cast<uintptr_t>(g_arena) + g_gkNext; g_gkNext += (static_cast<uintptr_t>(n) + 15) & ~15u; return p; }
static uintptr_t g_rectFn = 0, g_rectCtl = 0; static int g_rect[4], g_rectCalls = 0;
static void fakeSetRect(uint32_t fn, uintptr_t ctl, int* r) { ++g_rectCalls; g_rectFn = fn; g_rectCtl = ctl; for (int k = 0; k < 4; ++k) g_rect[k] = r[k]; }
static int g_nLoad = 0; static char g_loadName[24];
static void* fakeLoad(const char* n) { ++g_nLoad; snprintf(g_loadName, sizeof g_loadName, "%s", n); return reinterpret_cast<void*>(0x7000u + 8u * static_cast<unsigned>(g_nLoad)); }
static int g_exoCtors = 0, g_exoDtors = 0, g_textSets = 0, g_colors = 0; static uintptr_t g_textSubSeen = 0, g_colSubSeen = 0; static float g_colSeen[3];
static void* fakeExoCtor(void*, const char*) { ++g_exoCtors; return nullptr; }
static void fakeExoDtor(void*) { ++g_exoDtors; }
static void fakeTextSet(void* sub, void*) { ++g_textSets; g_textSubSeen = reinterpret_cast<uintptr_t>(sub); }
static void fakeTextColor(void* sub, float* v) { ++g_colors; g_colSubSeen = reinterpret_cast<uintptr_t>(sub); for (int k = 0; k < 3; ++k) g_colSeen[k] = v[k]; }
static void guikit_contract() {
    k2addr::setForTest(&kSteam);
    guikit::g_gk.rebind();
    guikit::Engine& e = guikit::g_gk;
    // ctlRect: extent at +4/+8/+0xC/+0x10 = x,y,w,h
    uintptr_t ctl = gkAlloc(0x200);
    at<int>(ctl, 4) = 10; at<int>(ctl, 8) = 20; at<int>(ctl, 0xC) = 30; at<int>(ctl, 0x10) = 40;
    int x = -1, y = -1, w = -1, h = -1;
    saferead::beginScope();
    HCHECK("getExtent reads x,y,w,h from ctl+4/+8/+0xC/+0x10 (ctlRect)", guikit::getExtent(ctl, &x, &y, &w, &h) && x == 10 && y == 20 && w == 30 && h == 40);
    x = y = w = h = -7;
    HCHECK("getExtent(null ctl) is false and leaves outputs untouched", !guikit::getExtent(0, &x, &y, &w, &h) && x == -7 && h == -7);
    HCHECK("getExtent on an unreadable ctl is false", !guikit::getExtent(0x20, &x, &y, &w, &h));
    // setCtlRect: vtable slot 1 (vt+4) called with {x,y,w,h}
    uintptr_t vt = gkAlloc(0x20);
    at<uint32_t>(ctl, 0) = static_cast<uint32_t>(vt); at<uint32_t>(vt, 4) = 0x00412345u;
    e.callSetRect = &fakeSetRect; g_rectCalls = 0;
    HCHECK("setRect calls vtable slot 1 (vt+4) with the ctl and {x,y,w,h}", guikit::setRect(ctl, 1, 2, 3, 4) && g_rectCalls == 1 && g_rectFn == 0x00412345u &&
           g_rectCtl == ctl && g_rect[0] == 1 && g_rect[1] == 2 && g_rect[2] == 3 && g_rect[3] == 4);
    at<uint32_t>(vt, 4) = 0; g_rectCalls = 0;
    HCHECK("setRect with a null slot-1 function does not call (the panel's old guard)", !guikit::setRect(ctl, 1, 2, 3, 4) && g_rectCalls == 0);
    at<uint32_t>(ctl, 0) = 0; g_rectCalls = 0;
    HCHECK("setRect with a null vtable does not call", !guikit::setRect(ctl, 1, 2, 3, 4) && g_rectCalls == 0);
    HCHECK("setRect(null ctl) does not call", !guikit::setRect(0, 1, 2, 3, 4) && g_rectCalls == 0);
    // textObjOk: label+0xD8 vtable 0x9876B4 and a non-null renderer at text+0x14
    uintptr_t lbl = gkAlloc(0x148);
    HCHECK("textObjOk false while the text vtable is wrong", !guikit::detail::textObjOk(lbl));
    at<uint32_t>(lbl, 0xD8) = 0x009876B4u;
    HCHECK("textObjOk false with the right vtable but no renderer", !guikit::detail::textObjOk(lbl));
    at<uint32_t>(lbl, 0xD8 + 0x14) = 0x1234u;
    HCHECK("textObjOk true: text vtable 0x9876B4 + renderer at +0x14", guikit::detail::textObjOk(lbl));
    // setText / setTextColor: engine calls on label+0xD8+0x18, and refused before the gate passes
    e.exoCtor = &fakeExoCtor; e.exoDtor = &fakeExoDtor; e.textSet = &fakeTextSet; e.textColor = &fakeTextColor;
    HCHECK("setText constructs, sets on label+0xF0, destroys", guikit::setText(lbl, "hi") && g_exoCtors == 1 && g_textSets == 1 && g_exoDtors == 1 && g_textSubSeen == lbl + 0xF0);
    HCHECK("setTextColor passes rgb to label+0xF0", guikit::setTextColor(lbl, 0.25f, 0.5f, 0.75f) && g_colors == 1 && g_colSubSeen == lbl + 0xF0 &&
           g_colSeen[0] == 0.25f && g_colSeen[1] == 0.5f && g_colSeen[2] == 0.75f);
    at<uint32_t>(lbl, 0xD8) = 0;
    HCHECK("setText / setTextColor refuse (no engine call) when the text object is bad", !guikit::setText(lbl, "x") && !guikit::setTextColor(lbl, 1, 1, 1) && g_textSets == 1 && g_colors == 1);
    // setFill / getBorderTint / setBorderTint: gated on the border vtable 0x9875BC; fill at border+0x74, tint at border+0x28
    HCHECK("setFill refused while the border vtable is wrong", !guikit::setFill(lbl, reinterpret_cast<void*>(0x5555)) && at<uint32_t>(lbl, 0x60 + 0x74) == 0);
    at<uint32_t>(lbl, 0x60) = 0x009875BCu;
    const float tint[3] = {0.5f, 0.25f, 1.0f}; float got[3] = {9, 9, 9};
    HCHECK("setFill (no tint) writes only the image at border+0x74", guikit::setFill(lbl, reinterpret_cast<void*>(0x5555)) && at<uint32_t>(lbl, 0x60 + 0x74) == 0x5555u && at<float>(lbl, 0x60 + 0x28) == 0.f);
    HCHECK("setBorderTint writes rgb at border+0x28 and getBorderTint reads it back", guikit::setBorderTint(lbl, tint) && guikit::getBorderTint(lbl, got) &&
           got[0] == 0.5f && got[1] == 0.25f && got[2] == 1.0f);
    HCHECK("setFill(nullptr) clears the image (an entry with no icon)", guikit::setFill(lbl, nullptr) && at<uint32_t>(lbl, 0x60 + 0x74) == 0);
    // imageFor: lowercased, pooled per resref, the loaded flag says "log this one" (the panel logs 'image ...' only then)
    e.loadImage = &fakeLoad; g_nLoad = 0;
    bool loaded = true;
    HCHECK("imageFor(null/empty) is nullptr and never loads", guikit::imageFor(nullptr, &loaded) == nullptr && !loaded && guikit::imageFor("") == nullptr && g_nLoad == 0);
    void* a = guikit::imageFor("IP_Mind", &loaded);
    HCHECK("first imageFor loads the LOWERCASE resref and reports loaded", a != nullptr && loaded && g_nLoad == 1 && !strcmp(g_loadName, "ip_mind"));
    void* a2 = guikit::imageFor("ip_mind", &loaded);
    HCHECK("repeat imageFor (any case) returns the pooled image, no reload, loaded=false (no log)", a2 == a && !loaded && g_nLoad == 1);
    void* b = guikit::imageFor("ii_drdutldev_003", &loaded);
    HCHECK("a 16-char resref reaches the loader untruncated", b != nullptr && b != a && loaded && !strcmp(g_loadName, "ii_drdutldev_003"));
    int pc = 0; guikit::imageFor("ip_mind", &loaded, &pc);
    HCHECK("pool capacity (256) covers more than the panel's old 128 entries", pc >= 2 && guikit::ImagePoolSize >= 128);
    e.loadImage = nullptr;
    HCHECK("unknown exe (no loader): imageFor returns nullptr, not a call into address 0", guikit::imageFor("ip_never_loaded") == nullptr);
    guikit::g_gk.rebind();
}

// ============================================================ 3. WindowHook<WheelTag>
struct WheelTag {};
using WheelHook = kp::WindowHook<WheelTag>;
static int g_handled = 0;
static intptr_t wheelProc(HWND h, unsigned m, intptr_t w, intptr_t l) {   // mirrors the mod's wheelProc: handle when active(), else forward
    if (WheelHook::active() && m == 0x20A) { ++g_handled; return 0; }
    return WheelHook::forward(h, m, w, l);
}
// a second DLL's subclass stacked above ours
static int g_otherHits = 0; static fakewnd::Proc g_otherPrev = nullptr;
static intptr_t otherProc(HWND h, unsigned m, intptr_t w, intptr_t l) { ++g_otherHits; return fakewnd::FakeCallWindowProc(g_otherPrev, h, m, w, l); }
static void resetWorld() {
    fakewnd::Reset();
    kp::whapi::pinCalls = kp::whapi::setCalls = kp::whapi::defHits = 0;
    WheelHook::old = nullptr; WheelHook::hwnd = HWND(); WheelHook::passthrough = 0; WheelHook::pins = 0;
    g_handled = g_otherHits = 0; g_otherPrev = nullptr;
}
static void windowhook_contract() {
    using namespace fakewnd;
    resetWorld();
    HCHECK("fresh hook is not active() and old is null (the mod installs only when !active())", !WheelHook::active() && WheelHook::old == nullptr);

    HCHECK("install on the game window succeeds", WheelHook::install(kHwnd, &wheelProc));
    HCHECK("install sets old to the previous proc (the mod logs it) and active() becomes true", WheelHook::old == &BaseGameProc && WheelHook::active());
    HEQ("install pins the DLL image once", kp::whapi::pinCalls, 1);
    HEQ("install subclasses with exactly one setProc", kp::whapi::setCalls, 1);
    HCHECK("our proc is on top of the window", FakeGetWindowLongPtr(kHwnd) == &wheelProc);

    WheelHook::install(kHwnd, &wheelProc);
    HEQ("second install while old is set makes no extra setProc", kp::whapi::setCalls, 1);
    HEQ("second install does not pin again", kp::whapi::pinCalls, 1);

    int b0 = base().hits;
    intptr_t r = WheelHook::forward(kHwnd, 0x200, 0, 41);
    HCHECK("forward reaches the previous proc (non-wheel messages keep working)", base().hits == b0 + 1 && r == 42);
    HCHECK("forward does not touch defProc while old is set", kp::whapi::defHits == 0);
    FakeSendMessage(0x20A, 0, 0);
    HEQ("active proc handles wheel messages itself", g_handled, 1);

    WheelHook::uninstall(&wheelProc);
    HCHECK("uninstall while on top restores the previous proc", FakeGetWindowLongPtr(kHwnd) == &BaseGameProc);
    HCHECK("uninstall while on top nulls old and hwnd", WheelHook::old == nullptr && WheelHook::hwnd == HWND());
    HCHECK("after top uninstall active() is false", !WheelHook::active());

    // 6: another subclass on top at detach -> passthrough
    resetWorld();
    WheelHook::install(kHwnd, &wheelProc);
    g_otherPrev = FakeSetWindowLongPtr(kHwnd, &otherProc);
    WheelHook::uninstall(&wheelProc);
    HCHECK("uninstall under another subclass leaves that subclass on top", FakeGetWindowLongPtr(kHwnd) == &otherProc);
    HCHECK("uninstall under another subclass sets passthrough: active() false", !WheelHook::active() && WheelHook::passthrough == 1);
    HCHECK("passthrough keeps old so the chain stays intact", WheelHook::old == &BaseGameProc);
    int bb = base().hits, hh = g_handled;
    intptr_t r2 = FakeSendMessage(0x20A, 0, 5);
    HCHECK("passthrough forward still chains to the previous proc and handles nothing", base().hits == bb + 1 && r2 == 6 && g_handled == hh && g_otherHits == 1);

    // 7: re-install clears passthrough
    int sets = kp::whapi::setCalls;
    WheelHook::install(kHwnd, &wheelProc);
    HCHECK("re-install after passthrough makes active() true again", WheelHook::active() && WheelHook::passthrough == 0);
    HCHECK("re-install after passthrough makes no extra setProc", kp::whapi::setCalls == sets);
    FakeSendMessage(0x20A, 0, 5);
    HEQ("re-installed proc handles wheel messages again", g_handled, hh + 1);
}

int main() {
    void* m = mmap(nullptr, 1u << 17, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    saferead_contract();
    effecticons_contract();
    guikit_contract();
    windowhook_contract();
    return kt::Report("common_contract_test (active_effects_panel)");
}
