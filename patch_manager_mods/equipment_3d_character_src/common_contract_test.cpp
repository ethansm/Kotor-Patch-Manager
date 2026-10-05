// common_contract_test.cpp -- Patches/Common CONTRACT test for the Equipment 3D Character preview (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/windowhook_test.cpp). Every message starts with "equip3d relies on: "
// so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to GameAddr.h /
// WindowHook.h re-runs this test: a Common change that would break the preview fails here first.
//
// Areas: (1) gameaddr constants (compile time), (2) saferead (host stub: every address reads as committed), (3) WindowHook as the mouse-wheel subclass: install only while old is null, handle
// only while active, forward chains to the previous proc, top uninstall restores, buried uninstall goes pass-through. Host-only.
// (4) sceneview on a fake engine in a MAP_32BIT arena (createView / initScene / addModel / camera / cameraAttach / cameraFov / gateOpen / modelCount).
#include "check.h"
#include "fake_wndproc.h"
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
using LONG = long;

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
#include "GameAddr.h"
#include "SafeRead.h"
#include "K2Addr.h"
#include "SceneView.h"

#define HCHECK(why, expr) ::kt::Check("equip3d relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("equip3d relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

// ============================================================ 1. GameAddr (compile time)
#define HADDR(name, val) static_assert(gameaddr::name == (val), "equip3d relies on: gameaddr::" #name " == " #val " (validated Steam address)")
HADDR(ClientCreatureVtable, 0x0099EE14);
HADDR(SceneCameraVtable, 0x0098C45C);
#define KADDR(f, val) static_assert(kSteam.f == (val), "equip3d relies on: kSteam." #f " == " #val " (original equip3d literal)")
KADDR(GuiGateGlobal, 0x009F42B4); KADDR(GuiControlCtor, 0x00418990); KADDR(GuiSceneCtor, 0x00417F40);
KADDR(SceneAddModel, 0x00418360); KADDR(SceneViewVtable, 0x009A3D34); KADDR(SceneCameraVtable, 0x0098C45C);
KADDR(SceneViewSize, 0x400); KADDR(View_Scene, 0x60); KADDR(View_SceneObj, 0x74); KADDR(View_Camera, 0x78); KADDR(View_ModelCount, 0x80);
KADDR(SceneVt_Init, 0x70); KADDR(SceneCamVt_Attach, 0x74); KADDR(SceneCamVt_Fov, 0x44);
static_assert(sceneview::SF_None == 0, "equip3d relies on: SF_None == 0");

// ============================================================ 2. SafeRead (probes replace IsBadReadPtr; DB lesson 234)
static void saferead_contract() {
    constexpr unsigned kSite = 0x0500;
    g_stubVirtualQueryAll = true;   // host stub: every address is one committed read-write region
    saferead::beginScope();
    HCHECK("saferead::beginScope is callable (every hook entry and after engine alloc/free)", true);
    HCHECK("probeRead refuses nullptr", !saferead::probeRead(kSite, nullptr, 4));
    HCHECK("probeRead refuses an address below 0x10000", !saferead::probeRead(kSite, reinterpret_cast<const void*>(0x1000), 4));
    volatile int cell = 0x1234ABCD;
    HCHECK("probeRead accepts readable memory", saferead::probeRead(kSite, const_cast<const int*>(&cell), 4));
    int v = 7;
    HCHECK("readAt<int> on a stack int (off 0) succeeds", saferead::readAt<int>(kSite, reinterpret_cast<uintptr_t>(&cell), 0, &v));
    HEQ("readAt<int> delivers the value", (long long)v, 0x1234ABCDll);
    v = 7;
    HCHECK("readAt refuses base 0 (the wrapper no longer needs its own base==0 guard)", !saferead::readAt<int>(kSite, 0, 0, &v));
    HEQ("readAt failure leaves *out untouched", (long long)v, 7ll);
    saferead::beginScope();
    HCHECK("reads still work after a second beginScope", saferead::readAt<int>(kSite, reinterpret_cast<uintptr_t>(&cell), 0, &v));
}

// ============================================================ 3. WindowHook as the wheel subclass
struct EquipTestTag {};
using Hook = kp::WindowHook<EquipTestTag>;
static int g_handled = 0;

// Mirrors wheelProc: handle only while active(), always forward the rest.
static intptr_t wheelProc(HWND h, unsigned m, intptr_t w, intptr_t l) {
    if (Hook::active()) ++g_handled;
    return Hook::forward(h, m, w, l);
}
// A second subclass of the game window that sits on top of ours.
static intptr_t otherProc(HWND, unsigned, intptr_t, intptr_t l) { return l + 100; }

static void resetWorld() {
    fakewnd::Reset();
    kp::whapi::pinCalls = kp::whapi::setCalls = kp::whapi::defHits = 0;
    Hook::old = nullptr; Hook::hwnd = HWND(); Hook::passthrough = 0; Hook::pins = 0; g_handled = 0;
}

static void windowhook_contract() {
    using namespace fakewnd;
    resetWorld();
    // (1) fresh
    HCHECK("fresh hook: active() is false", !Hook::active());
    HCHECK("fresh hook: old is null (the install guard is !old)", Hook::old == nullptr);

    // (2) install
    HCHECK("install on the game window succeeds", Hook::install(kHwnd, &wheelProc));
    HCHECK("install records the previous proc in old", Hook::old == &BaseGameProc);
    HCHECK("active() is true after install", Hook::active());
    HEQ("install pins the DLL image once", kp::whapi::pinCalls, 1);
    HEQ("install calls setProc once", kp::whapi::setCalls, 1);
    HCHECK("our proc is the top proc after install", FakeGetWindowLongPtr(kHwnd) == &wheelProc);

    // (3) MANDATORY: a second install while old is set must not subclass again (equip3d re-installs only when old is null)
    Hook::install(kHwnd, &wheelProc);
    HEQ("second install while old set makes no extra setProc", kp::whapi::setCalls, 1);
    HCHECK("second install keeps old unchanged", Hook::old == &BaseGameProc);
    HEQ("second install does not pin again", kp::whapi::pinCalls, 1);

    // (4) forward reaches the previous proc
    int hitsBefore = base().hits;
    intptr_t r = FakeSendMessage(0x20A, 7, 41);
    HCHECK("wheel proc handles the message while active", g_handled == 1);
    HCHECK("forward reaches the previous proc exactly once", base().hits == hitsBefore + 1);
    HEQ("forward returns the previous proc result", (long long)r, 42ll);

    // (5) uninstall while on top restores the previous proc and nulls old
    Hook::uninstall(&wheelProc);
    HCHECK("top uninstall restores the previous proc", FakeGetWindowLongPtr(kHwnd) == &BaseGameProc);
    HCHECK("top uninstall nulls old (so a later install can subclass again)", Hook::old == nullptr);
    HCHECK("top uninstall: active() is false", !Hook::active());
    HEQ("top uninstall leaves passthrough clear", (long long)Hook::passthrough, 0ll);

    // (6) uninstall while another subclass is on top -> pass-through, still chained
    resetWorld();
    Hook::install(kHwnd, &wheelProc);
    WNDPROC below = FakeSetWindowLongPtr(kHwnd, &otherProc);
    HCHECK("setup: other subclass chains over ours", below == &wheelProc);
    Hook::uninstall(&wheelProc);
    HCHECK("buried uninstall: active() is false", !Hook::active());
    HEQ("buried uninstall sets passthrough", (long long)Hook::passthrough, 1ll);
    HCHECK("buried uninstall keeps old so the chain stays intact", Hook::old == &BaseGameProc);
    HCHECK("buried uninstall leaves the other proc on top", FakeGetWindowLongPtr(kHwnd) == &otherProc);
    // the other proc forwards to ours (as a chained subclass would): we must not handle, only chain to the base
    int baseBefore = base().hits, handledBefore = g_handled;
    intptr_t r2 = wheelProc(kHwnd, 0x20A, 7, 41);
    HCHECK("pass-through: our proc does not handle", g_handled == handledBefore);
    HCHECK("pass-through: forward still chains to the previous proc", base().hits == baseBefore + 1);
    HEQ("pass-through: forward result is the previous proc result", (long long)r2, 42ll);
    HEQ("pass-through: no default-proc fallback", kp::whapi::defHits, 0);
}

// ============================================================ 4. SceneView (fake engine in a MAP_32BIT arena)
#define U(x) static_cast<unsigned long long>(x)
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a;
}
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static int nAlloc, nCtl, nScene, nInit, nExoCtor, nExoDtor, nAdd, nSceneInit, nCamAttach, nCamFov;
static unsigned lastAllocSize; static bool allocFails;
static void* lastCtlThis; static void* lastSceneThis; static uint32_t vtAtCtlCtor;
static void* lastInitPanel; static void* lastInitView; static char lastInitTag[16]; static int lastInitA, lastInitB;
static void* lastAddThis; static char lastAddRes[16]; static int lastAddIdx;
static uint32_t siFn; static uintptr_t siScene; static char siRoom[32];
static uint32_t caFn, cfFn; static uintptr_t caCam, cfCam; static void* caModel; static char caNode[16]; static int caFlag; static float cfFov;

static void* fAlloc(unsigned n) { ++nAlloc; lastAllocSize = n; if (allocFails) return nullptr; uintptr_t a = arenaAlloc(n); memset(reinterpret_cast<void*>(a), 0xAB, n); return reinterpret_cast<void*>(a); }
static void fCtl(void* t) { ++nCtl; lastCtlThis = t; vtAtCtlCtor = *static_cast<uint32_t*>(t); }
static void fScene(void* t) { ++nScene; lastSceneThis = t; }
static void fInitCtl(void* panel, void* view, void* exo, int a, int b) { ++nInit; lastInitPanel = panel; lastInitView = view; strncpy(lastInitTag, static_cast<char*>(exo), 15); lastInitA = a; lastInitB = b; }
static void* fExoCtor(void* e, const char* s) { ++nExoCtor; memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fExoDtor(void*) { ++nExoDtor; }
static void* fAdd(void* t, void* exo, int idx) { ++nAdd; lastAddThis = t; strncpy(lastAddRes, static_cast<char*>(exo), 15); lastAddIdx = idx; return reinterpret_cast<void*>(0x7001); }
static void fSceneInit(uint32_t fn, uintptr_t sc, const char* room, float*, float*) { ++nSceneInit; siFn = fn; siScene = sc; strncpy(siRoom, room, 31); }
static void fCamAttach(uint32_t fn, uintptr_t cam, void* m, const char* node, int flag) { ++nCamAttach; caFn = fn; caCam = cam; caModel = m; strncpy(caNode, node, 15); caFlag = flag; }
static void fCamFov(uint32_t fn, uintptr_t cam, float fov) { ++nCamFov; cfFn = fn; cfCam = cam; cfFov = fov; }

static void resetCounters() {
    nAlloc = nCtl = nScene = nInit = nExoCtor = nExoDtor = nAdd = nSceneInit = nCamAttach = nCamFov = 0;
    allocFails = false; lastAllocSize = 0; lastCtlThis = lastSceneThis = lastInitPanel = lastInitView = nullptr; lastInitTag[0] = 0; lastInitA = lastInitB = 0; vtAtCtlCtor = 0;
}
static void installFakes() {
    k2addr::clearForTest(); sceneview::g_sv.rebind();
    auto& e = sceneview::g_sv;
    e.alloc = fAlloc; e.controlCtor = fCtl; e.sceneCtor = fScene; e.initControl = fInitCtl; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.addModel = fAdd;
    e.callSceneInit = fSceneInit; e.callCamAttach = fCamAttach; e.callCamFov = fCamFov;
}

static void sceneview_contract() {
    using namespace sceneview;
    // ---- gateOpen: int at GuiGateGlobal (BindEquipView logs the int and refuses a zero gate) ----
    installFakes();
    uintptr_t cell = arenaAlloc(16);
    K2AddrTable t = kSteam; t.GuiGateGlobal = cell;
    k2addr::setForTest(&t);
    int gate = 99;
    at<int>(cell, 0) = 0; HCHECK("gateOpen false while the gate int is 0", !gateOpen(&gate)); HEQ("gateOpen reports 0", (long long)gate, 0ll);
    at<int>(cell, 0) = 7; HCHECK("gateOpen true when the gate int is non-zero", gateOpen(&gate)); HEQ("gateOpen reports the int read (the mod logs it)", (long long)gate, 7ll);

    // ---- createView (BindEquipView: 3D_MODEL control of the equip panel) ----
    installFakes(); resetCounters();
    uintptr_t panel = arenaAlloc(0x100);
    unsigned char* v = createView(panel, "3D_MODEL");
    HCHECK("createView returns the allocated block", v != nullptr);
    HEQ("createView allocates SceneViewSize 0x400 bytes", U(lastAllocSize), 0x400ull);
    HEQ("createView zeroes the block before the control ctor", U(vtAtCtlCtor), 0ull);
    bool zero = true; for (int i = 4; i < 0x400; ++i) if (v && v[i]) zero = false;
    HCHECK("createView block is fully zeroed (fake alloc fills 0xAB)", zero);
    HCHECK("createView writes the scene-view vtable 0x9A3D34 at +0", v && at<uint32_t>((uintptr_t)v, 0) == 0x9A3D34);
    HCHECK("createView runs the scene ctor on view+0x60", nScene == 1 && lastSceneThis == v + 0x60 && nCtl == 1 && lastCtlThis == v);
    HCHECK("createView calls initControl(panel, view, tag, addToList=1, scale=1)",
           nInit == 1 && lastInitPanel == reinterpret_cast<void*>(panel) && lastInitView == v && !strcmp(lastInitTag, "3D_MODEL") && lastInitA == 1 && lastInitB == 1);
    HCHECK("createView balances the CExoString ctor/dtor", nExoCtor == 1 && nExoDtor == 1);
    resetCounters(); allocFails = true;
    HCHECK("createView returns null when operator new fails (mod logs 'operator new failed')", createView(panel, "3D_MODEL") == nullptr);
    HCHECK("createView after a failed alloc runs no ctor / initControl", nAlloc == 1 && nCtl + nScene + nInit + nExoCtor == 0);

    // ---- initScene ----
    installFakes(); resetCounters();
    uintptr_t view = arenaAlloc(0x400), scene = arenaAlloc(0x40), svt = arenaAlloc(0x100);
    at<uint32_t>(svt, 0x70) = 0xCAFE0001u; at<uint32_t>(scene, 0) = (uint32_t)svt; at<uint32_t>(view, 0x74) = (uint32_t)scene;
    void* vp = reinterpret_cast<void*>(view);
    uintptr_t so = 99;
    HEQ("initScene gui3D_room -> SF_None", (long long)initScene(vp, "gui3D_room", &so), (long long)SF_None);
    HCHECK("initScene reports the Scene*, calls vtable +0x70 with the room", so == scene && nSceneInit == 1 && siFn == 0xCAFE0001u && siScene == scene && !strcmp(siRoom, "gui3D_room"));
    at<uint32_t>(view, 0x74) = 0; so = 99;
    HCHECK("initScene null Scene* is a non-None, non-VtableBad code (mod logs 'scene pointer bad')", initScene(vp, "gui3D_room", &so) == SF_SceneBad);
    HEQ("initScene null Scene* reports *sceneOut == 0 (mod logs it)", U(so), 0ull);
    at<uint32_t>(view, 0x74) = (uint32_t)scene; at<uint32_t>(scene, 0) = 0x20;
    HEQ("initScene unreadable scene vtable -> SF_VtableBad (mod logs 'scene vtable bad')", (long long)initScene(vp, "gui3D_room"), (long long)SF_VtableBad);
    HEQ("initScene refusals make no engine call", (long long)nSceneInit, 1ll);
    at<uint32_t>(scene, 0) = (uint32_t)svt;

    // ---- model count / addModel (the light rig) ----
    at<int>(view, 0x80) = 3;
    HEQ("modelCount reads view+0x80", (long long)modelCount(vp), 3ll);
    HEQ("modelCount of a null view is -1", (long long)modelCount(nullptr), -1ll);
    HCHECK("addModel(view, rig, -1) passes view+0x60, resref, idx -1", addModel(vp, "p_rig", -1) == reinterpret_cast<void*>(0x7001) &&
           lastAddThis == reinterpret_cast<void*>(view + 0x60) && !strcmp(lastAddRes, "p_rig") && lastAddIdx == -1);
    HCHECK("addModel balances the CExoString ctor/dtor", nExoCtor == nExoDtor && nExoCtor == 1);

    // ---- camera ----
    uintptr_t cam = arenaAlloc(0x20); at<uint32_t>(cam, 0) = 0x98C45C; at<uint32_t>(view, 0x78) = (uint32_t)cam;
    uint32_t c = 0, cv = 0;
    HCHECK("camera true for the 0x98C45C vtable", camera(vp, &c, &cv)); HCHECK("camera fills cam and camVt (mod logs both)", c == cam && cv == 0x98C45C);
    at<uint32_t>(cam, 0) = 0x1234; c = cv = 0;
    HCHECK("camera false for a foreign vtable but still fills cam/camVt", !camera(vp, &c, &cv) && c == cam && cv == 0x1234);

    // ---- cameraAttach / cameraFov (camera vtable slots +0x74 / +0x44; the real camera vtable is not mappable, so the table's
    //      expectation is pointed at an arena vtable) ----
    installFakes(); resetCounters();
    uintptr_t cvt = arenaAlloc(0x100), cam2 = arenaAlloc(0x20), view2 = arenaAlloc(0x400);
    at<uint32_t>(cvt, 0x74) = 0xA1A1A1A1u; at<uint32_t>(cvt, 0x44) = 0xB2B2B2B2u; at<uint32_t>(cam2, 0) = (uint32_t)cvt; at<uint32_t>(view2, 0x78) = (uint32_t)cam2;
    void* v2 = reinterpret_cast<void*>(view2); char model[4];
    K2AddrTable ct = kSteam; ct.SceneCameraVtable = cvt; k2addr::setForTest(&ct);
    HCHECK("cameraAttach calls camera vtable +0x74 with (cam, rig, node, 1)", cameraAttach(v2, model, "camerahook", 1) && nCamAttach == 1 && caFn == 0xA1A1A1A1u &&
           caCam == cam2 && caModel == model && !strcmp(caNode, "camerahook") && caFlag == 1);
    HCHECK("cameraFov calls camera vtable +0x44 with (cam, fov)", cameraFov(v2, 42.5f) && nCamFov == 1 && cfFn == 0xB2B2B2B2u && cfCam == cam2 && cfFov == 42.5f);
    at<uint32_t>(cam2, 0) = 0x12345678u;
    HCHECK("camera helpers refuse a foreign camera vtable (applyView FOV is vtable-checked) and call nothing",
           !cameraAttach(v2, model, "camerahook", 1) && !cameraFov(v2, 1.f) && nCamAttach == 1 && nCamFov == 1);
    at<uint32_t>(cam2, 0) = (uint32_t)cvt;
    HCHECK("camera helpers refuse a null view", !cameraAttach(nullptr, model, "camerahook", 1) && !cameraFov(nullptr, 1.f) && nCamAttach == 1 && nCamFov == 1);
    K2AddrTable z = ct; z.SceneCamVt_Attach = 0; k2addr::setForTest(&z);
    HCHECK("cameraAttach refuses a zero slot offset (cameraFov unaffected)", !cameraAttach(v2, model, "camerahook", 1) && nCamAttach == 1 && cameraFov(v2, 1.f));
    z = ct; z.SceneCamVt_Fov = 0; k2addr::setForTest(&z);
    HCHECK("cameraFov refuses a zero slot offset", !cameraFov(v2, 1.f) && nCamFov == 2);
    k2addr::setForTest(nullptr);
    HCHECK("camera helpers refuse a null K2Addr table (unknown exe)", !cameraAttach(v2, model, "camerahook", 1) && !cameraFov(v2, 1.f) && !createView(panel, "3D_MODEL") && nCamAttach == 1 && nCamFov == 2);
    installFakes();
}

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    saferead_contract();
    windowhook_contract();
    sceneview_contract();
    return kt::Report("common_contract_test (equipment_3d_character)");
}
