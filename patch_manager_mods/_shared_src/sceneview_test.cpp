// Host test of SceneView.h: the view-block recipe, scene init, model calls, camera check and the Gob vtable guard / pose helpers.
// Every engine call is a function pointer in sceneview::g_sv; the fakes below record their arguments. Engine pointers live in a
// MAP_32BIT arena (vtable slots and Scene* are 32-bit fields). Unreadable memory = any address below 0x10000 (saferead refuses it).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SceneView.h"
#include "check.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

using namespace sceneview;
static const K2AddrTable kZero{};   // an unfilled table (does not depend on how far the GOG column is filled)
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) { n = (n + 15) & ~size_t(15); if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); } uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
#define U(x) static_cast<unsigned long long>(x)
#define P(x) U(reinterpret_cast<uintptr_t>(x))

// ---- fakes / recorders ----
static int nAlloc, nCtl, nScene, nInit, nExoCtor, nExoDtor, nAdd, nRem, nGet, nSceneInit, nSetPos, nSetOri, nGetVec, nGetParent;
static unsigned lastAllocSize; static bool allocFails;
static void* lastCtlThis; static void* lastSceneThis;
static void* lastInitPanel; static void* lastInitView; static char lastInitTag[16]; static int lastInitA, lastInitB; static void* lastInitExo;
static void* lastDtorExo, *lastExoCtorExo; static uint32_t vtAtCtlCtor;
static void* lastAddThis; static char lastAddRes[16]; static int lastAddIdx; static void* addResult = reinterpret_cast<void*>(0x7001);
static void* lastRemThis; static int lastRemIdx, lastRemDel;
static void* lastGetThis; static int lastGetIdx; static void* getResult = reinterpret_cast<void*>(0x7002);
static uint32_t siFn; static uintptr_t siScene; static const char* siRoom; static float siPos[3], siQuat[4];
static uint32_t spFn; static void* spGob; static float spV[3];
static uint32_t soFn; static void* soGob; static float soV[4];
static uint32_t gvFn[4]; static void* gvGob[4]; static float gvOut0[4];
static int nCamAttach, nCamFov; static uint32_t caFn, cfFn; static uintptr_t caCam, cfCam; static void* caModel; static const char* caNode; static int caFlag; static float cfFov;
static uint32_t gpFn; static void* gpGob; static int gpArg; static void* gpResult = reinterpret_cast<void*>(0x7003);

static void* fAlloc(unsigned n) { ++nAlloc; lastAllocSize = n; if (allocFails) return nullptr; uintptr_t a = arenaAlloc(n); memset(reinterpret_cast<void*>(a), 0xAB, n); return reinterpret_cast<void*>(a); }
static void fCtl(void* t) { ++nCtl; lastCtlThis = t; vtAtCtlCtor = *static_cast<uint32_t*>(t); }
static void fScene(void* t) { ++nScene; lastSceneThis = t; }
static void fInitCtl(void* panel, void* view, void* exo, int a, int b) { ++nInit; lastInitPanel = panel; lastInitView = view; lastInitExo = exo; strncpy(lastInitTag, static_cast<char*>(exo), 15); lastInitA = a; lastInitB = b; }
static void* fExoCtor(void* e, const char* s) { ++nExoCtor; lastExoCtorExo = e; memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fExoDtor(void* e) { ++nExoDtor; lastDtorExo = e; }
static void* fAdd(void* t, void* exo, int idx) { ++nAdd; lastAddThis = t; strncpy(lastAddRes, static_cast<char*>(exo), 15); lastAddIdx = idx; return addResult; }
static void fRem(void* t, int idx, int del) { ++nRem; lastRemThis = t; lastRemIdx = idx; lastRemDel = del; }
static void* fGet(void* t, int idx) { ++nGet; lastGetThis = t; lastGetIdx = idx; return getResult; }
static void fSceneInit(uint32_t fn, uintptr_t sc, const char* room, float* pos, float* quat) { ++nSceneInit; siFn = fn; siScene = sc; siRoom = room; memcpy(siPos, pos, sizeof siPos); memcpy(siQuat, quat, sizeof siQuat); }
static void fSetPos(uint32_t fn, void* g, float x, float y, float z) { ++nSetPos; spFn = fn; spGob = g; spV[0] = x; spV[1] = y; spV[2] = z; }
static void fSetOri(uint32_t fn, void* g, float w, float x, float y, float z) { ++nSetOri; soFn = fn; soGob = g; soV[0] = w; soV[1] = x; soV[2] = y; soV[3] = z; }
static void fGetVec(uint32_t fn, void* g, float* out) {
    gvFn[nGetVec & 3] = fn; gvGob[nGetVec & 3] = g;
    if (nGetVec == 0) { out[0] = 1.5f; out[1] = 2.5f; out[2] = 3.5f; } else { out[0] = 0.5f; out[1] = 0.25f; out[2] = 0.125f; out[3] = 0.0625f; }
    ++nGetVec; (void)gvOut0;
}
static void fCamAttach(uint32_t fn, uintptr_t cam, void* m, const char* node, int flag) { ++nCamAttach; caFn = fn; caCam = cam; caModel = m; caNode = node; caFlag = flag; }
static void fCamFov(uint32_t fn, uintptr_t cam, float fov) { ++nCamFov; cfFn = fn; cfCam = cam; cfFov = fov; }
static void* fGetParent(uint32_t fn, void* g, int arg) { ++nGetParent; gpFn = fn; gpGob = g; gpArg = arg; return gpResult; }

static void resetCounters() {
    nAlloc = nCtl = nScene = nInit = nExoCtor = nExoDtor = nAdd = nRem = nGet = nSceneInit = nSetPos = nSetOri = nGetVec = nGetParent = nCamAttach = nCamFov = 0;
    allocFails = false; lastAllocSize = 0; lastCtlThis = lastSceneThis = lastInitPanel = lastInitView = lastInitExo = lastDtorExo = lastExoCtorExo = nullptr;
    lastInitTag[0] = 0; lastInitA = lastInitB = 0; vtAtCtlCtor = 0;
}
static void installFakes() {
    g_sv.rebind();
    g_sv.alloc = fAlloc; g_sv.controlCtor = fCtl; g_sv.sceneCtor = fScene; g_sv.initControl = fInitCtl; g_sv.exoCtor = fExoCtor; g_sv.exoDtor = fExoDtor;
    g_sv.addModel = fAdd; g_sv.removeModel = fRem; g_sv.getModel = fGet;
    g_sv.callSceneInit = fSceneInit; g_sv.callSetPos = fSetPos; g_sv.callSetOri = fSetOri; g_sv.callGetVec = fGetVec; g_sv.callGetParent = fGetParent;
    g_sv.callCamAttach = fCamAttach; g_sv.callCamFov = fCamFov;
}

// fake Gob + vtable in the arena; slot values from the Steam table
static const uint32_t kSlotVal[GobSlotCount] = { 0x00461490, 0x004614E0, 0x00459710, 0x00459740, 0x00461530 };
static const int kSlotOff[GobSlotCount] = { 0x5C, 0x60, 0x64, 0x68, 0x108 };
static uintptr_t makeVtable(int wrongIdx = -1, uint32_t wrongVal = 0) {
    uintptr_t vt = arenaAlloc(0x200);
    for (int i = 0; i < GobSlotCount; ++i) at<uint32_t>(vt, kSlotOff[i]) = (i == wrongIdx) ? wrongVal : kSlotVal[i];
    return vt;
}
static uintptr_t makeGob(uintptr_t vt) { uintptr_t g = arenaAlloc(0x300); at<uint32_t>(g, 0) = (uint32_t)vt; return g; }

static void testGate() {
    k2addr::clearForTest();
    KT_CHECK_EQ(U(k2addr::addr(&K2AddrTable::GuiGateGlobal)), 0x9F42B4ull);
    uintptr_t cell = arenaAlloc(16);
    K2AddrTable t = kSteam; t.GuiGateGlobal = cell;
    k2addr::setForTest(&t);
    int v = 99;
    at<int>(cell, 0) = 0; KT_CHECK(!gateOpen(&v)); KT_CHECK_EQ((long long)v, 0ll);
    at<int>(cell, 0) = 5; KT_CHECK(gateOpen(&v)); KT_CHECK_EQ((long long)v, 5ll);
    KT_CHECK(gateOpen());
    at<int>(cell, 0) = -1; KT_CHECK(gateOpen(&v) && v == -1);
    K2AddrTable low = kSteam; low.GuiGateGlobal = 0x20;      // unreadable
    k2addr::setForTest(&low); v = 99;
    KT_CHECK(!gateOpen(&v)); KT_CHECK_EQ((long long)v, 0ll);
    K2AddrTable none = kSteam; none.GuiGateGlobal = 0;
    k2addr::setForTest(&none); v = 99;
    KT_CHECK(!gateOpen(&v)); KT_CHECK_EQ((long long)v, 0ll);
    k2addr::setForTest(&kZero); KT_CHECK(!gateOpen());
    k2addr::setForTest(nullptr); KT_CHECK(!gateOpen());
    k2addr::clearForTest();
}

static void testRebind() {
    k2addr::clearForTest();
    Engine e;
    KT_CHECK_EQ(P(e.alloc), 0x919723ull); KT_CHECK_EQ(P(e.controlCtor), 0x418990ull); KT_CHECK_EQ(P(e.sceneCtor), 0x417F40ull);
    KT_CHECK_EQ(P(e.initControl), 0x40F620ull); KT_CHECK_EQ(P(e.exoCtor), 0x733570ull); KT_CHECK_EQ(P(e.exoDtor), 0x733780ull);
    KT_CHECK_EQ(P(e.addModel), 0x418360ull); KT_CHECK_EQ(P(e.removeModel), 0x418430ull); KT_CHECK_EQ(P(e.getModel), 0x4184B0ull);
    k2addr::setForTest(&kZero);
    Engine g;
    KT_CHECK(!g.alloc && !g.controlCtor && !g.sceneCtor && !g.initControl && !g.addModel && !g.removeModel && !g.getModel);
    k2addr::clearForTest();
}

static void testCreateView() {
    k2addr::clearForTest(); installFakes(); resetCounters();
    uintptr_t panel = arenaAlloc(0x100);
    unsigned char* v = createView(panel, "LB_3D");
    KT_CHECK(v != nullptr);
    KT_CHECK_EQ(U(lastAllocSize), 0x400ull); KT_CHECK_EQ((long long)nAlloc, 1ll);
    KT_CHECK_EQ((long long)nCtl, 1ll); KT_CHECK(lastCtlThis == v);
    KT_CHECK_EQ(U(vtAtCtlCtor), 0ull);                            // block was zeroed before the control ctor ran
    bool zero = true; for (int i = 4; i < 0x400; ++i) if (v && v[i]) zero = false;
    KT_CHECK(zero);                                               // (0xAB fill from the fake alloc was cleared)
    KT_CHECK_EQ(U(v ? at<uint32_t>((uintptr_t)v, 0) : 1), 0x9A3D34ull);
    KT_CHECK_EQ((long long)nScene, 1ll); KT_CHECK(lastSceneThis == v + 0x60);
    KT_CHECK_EQ((long long)nExoCtor, 1ll);
    KT_CHECK_EQ((long long)nInit, 1ll); KT_CHECK(lastInitPanel == reinterpret_cast<void*>(panel)); KT_CHECK(lastInitView == v);
    KT_CHECK(strcmp(lastInitTag, "LB_3D") == 0); KT_CHECK_EQ((long long)lastInitA, 1ll); KT_CHECK_EQ((long long)lastInitB, 1ll);
    KT_CHECK_EQ((long long)nExoDtor, 1ll); KT_CHECK(lastDtorExo == lastInitExo && lastDtorExo == lastExoCtorExo);

    resetCounters();
    KT_CHECK(createView(0, "LB_3D") == nullptr); KT_CHECK(createView(panel, nullptr) == nullptr);
    KT_CHECK_EQ((long long)(nAlloc + nCtl + nScene + nInit + nExoCtor), 0ll);

    resetCounters(); allocFails = true;
    KT_CHECK(createView(panel, "LB_3D") == nullptr);
    KT_CHECK_EQ((long long)nAlloc, 1ll); KT_CHECK_EQ((long long)(nCtl + nScene + nInit + nExoCtor + nExoDtor), 0ll);

    // a missing engine pointer refuses before allocating
    resetCounters(); auto savedInit = g_sv.initControl; g_sv.initControl = nullptr;
    KT_CHECK(createView(panel, "LB_3D") == nullptr); KT_CHECK_EQ((long long)nAlloc, 0ll);
    g_sv.initControl = savedInit;

    // unknown exe / unfilled table / zero size: no engine call
    resetCounters(); k2addr::setForTest(nullptr);
    KT_CHECK(createView(panel, "LB_3D") == nullptr);
    k2addr::setForTest(&kZero);
    KT_CHECK(createView(panel, "LB_3D") == nullptr);
    K2AddrTable noSize = kSteam; noSize.SceneViewSize = 0; k2addr::setForTest(&noSize);
    KT_CHECK(createView(panel, "LB_3D") == nullptr);
    K2AddrTable noVt = kSteam; noVt.SceneViewVtable = 0; k2addr::setForTest(&noVt);
    KT_CHECK(createView(panel, "LB_3D") == nullptr);
    K2AddrTable noScn = kSteam; noScn.View_Scene = 0; k2addr::setForTest(&noScn);
    KT_CHECK(createView(panel, "LB_3D") == nullptr);
    KT_CHECK_EQ((long long)(nAlloc + nCtl + nScene + nInit + nExoCtor + nExoDtor), 0ll);

    k2addr::clearForTest(); g_sv.rebind(); installFakes();
    resetCounters();
    KT_CHECK(createView(panel, "LB_3D") != nullptr); KT_CHECK_EQ((long long)nAlloc, 1ll);
}

// a fake view: Scene* at +0x74 -> scene object -> vtable with +0x70 = 0xCAFE0001
static uintptr_t makeView(uintptr_t* sceneOut = nullptr, uintptr_t* vtOut = nullptr) {
    uintptr_t view = arenaAlloc(0x400), scene = arenaAlloc(0x40), vt = arenaAlloc(0x100);
    at<uint32_t>(vt, 0x70) = 0xCAFE0001u; at<uint32_t>(scene, 0) = (uint32_t)vt; at<uint32_t>(view, 0x74) = (uint32_t)scene;
    if (sceneOut) *sceneOut = scene;
    if (vtOut) *vtOut = vt;
    return view;
}

static void testInitScene() {
    k2addr::clearForTest(); installFakes(); resetCounters();
    uintptr_t scene, vt; uintptr_t view = makeView(&scene, &vt);
    uintptr_t so = 99;
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "gui3D_room", &so), (long long)SF_None);
    KT_CHECK_EQ(U(so), U(scene)); KT_CHECK_EQ((long long)nSceneInit, 1ll);
    KT_CHECK_EQ(U(siFn), 0xCAFE0001ull); KT_CHECK_EQ(U(siScene), U(scene)); KT_CHECK(strcmp(siRoom, "gui3D_room") == 0);
    KT_CHECK(siPos[0] == 0 && siPos[1] == 0 && siPos[2] == 0);
    KT_CHECK(siQuat[0] == 0 && siQuat[1] == 0 && siQuat[2] == 0 && siQuat[3] == 1);
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r"), (long long)SF_None);   // sceneOut optional

    resetCounters();
    at<uint32_t>(view, 0x74) = 0; so = 99;
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r", &so), (long long)SF_SceneBad); KT_CHECK_EQ(U(so), 0ull);
    at<uint32_t>(view, 0x74) = 0x20; so = 99;                      // non-null but unreadable
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r", &so), (long long)SF_SceneBad); KT_CHECK_EQ(U(so), 0x20ull);
    at<uint32_t>(view, 0x74) = (uint32_t)scene;
    at<uint32_t>(scene, 0) = 0x20;                                 // vtable unreadable
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r"), (long long)SF_VtableBad);
    at<uint32_t>(scene, 0) = 0;                                    // null vtable
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r"), (long long)SF_VtableBad);
    KT_CHECK_EQ((long long)nSceneInit, 0ll);
    at<uint32_t>(scene, 0) = (uint32_t)vt;

    KT_CHECK_EQ((long long)initScene(nullptr, "r"), (long long)SF_Unknown);
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), nullptr), (long long)SF_Unknown);
    uintptr_t so2 = 99; k2addr::setForTest(nullptr);
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r", &so2), (long long)SF_Unknown); KT_CHECK_EQ(U(so2), 0ull);
    k2addr::setForTest(&kZero);
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r"), (long long)SF_Unknown);
    K2AddrTable noInit = kSteam; noInit.SceneVt_Init = 0; k2addr::setForTest(&noInit);
    KT_CHECK_EQ((long long)initScene(reinterpret_cast<void*>(view), "r"), (long long)SF_Unknown);
    KT_CHECK_EQ((long long)nSceneInit, 0ll);
    k2addr::clearForTest();
}

static void testModels() {
    k2addr::clearForTest(); installFakes(); resetCounters();
    uintptr_t view = makeView(); void* vp = reinterpret_cast<void*>(view);
    at<int>(view, 0x80) = 3;
    KT_CHECK_EQ((long long)modelCount(vp), 3ll); KT_CHECK_EQ((long long)modelCount(nullptr), -1ll);
    KT_CHECK_EQ((long long)modelCount(reinterpret_cast<void*>(0x20 - 0x80 + 0x80)), -1ll);   // unreadable view

    KT_CHECK(getModel(vp, 2) == getResult); KT_CHECK(lastGetThis == reinterpret_cast<void*>(view + 0x60)); KT_CHECK_EQ((long long)lastGetIdx, 2ll);
    KT_CHECK(getModel(nullptr, 0) == nullptr); KT_CHECK_EQ((long long)nGet, 1ll);

    KT_CHECK(addModel(vp, "p_bastilla") == addResult);
    KT_CHECK(lastAddThis == reinterpret_cast<void*>(view + 0x60)); KT_CHECK(strcmp(lastAddRes, "p_bastilla") == 0); KT_CHECK_EQ((long long)lastAddIdx, -1ll);
    KT_CHECK_EQ((long long)nExoCtor, 1ll); KT_CHECK_EQ((long long)nExoDtor, 1ll);
    KT_CHECK(addModel(vp, "x", 4) == addResult && lastAddIdx == 4);
    KT_CHECK(addModel(nullptr, "x") == nullptr); KT_CHECK(addModel(vp, nullptr) == nullptr); KT_CHECK_EQ((long long)nAdd, 2ll);
    addResult = nullptr; KT_CHECK(addModel(vp, "x") == nullptr); addResult = reinterpret_cast<void*>(0x7001);
    KT_CHECK_EQ((long long)nExoDtor, nExoCtor);

    KT_CHECK(removeModel(vp, 1)); KT_CHECK(lastRemThis == reinterpret_cast<void*>(view + 0x60) && lastRemIdx == 1 && lastRemDel == 1);
    KT_CHECK(removeModel(vp, 2, false)); KT_CHECK(lastRemIdx == 2 && lastRemDel == 0);
    KT_CHECK(!removeModel(nullptr, 0)); KT_CHECK_EQ((long long)nRem, 2ll);

    k2addr::setForTest(nullptr); int a = nGet, b = nAdd, c = nRem;
    KT_CHECK(getModel(vp, 0) == nullptr && addModel(vp, "x") == nullptr && !removeModel(vp, 0) && modelCount(vp) == -1);
    KT_CHECK(a == nGet && b == nAdd && c == nRem);
    k2addr::setForTest(&kZero);
    KT_CHECK(getModel(vp, 0) == nullptr && modelCount(vp) == -1);
    K2AddrTable noScene = kSteam; noScene.View_Scene = 0; k2addr::setForTest(&noScene);
    KT_CHECK(getModel(vp, 0) == nullptr && addModel(vp, "x") == nullptr && !removeModel(vp, 0));
    KT_CHECK_EQ((long long)(a - nGet + b - nAdd + c - nRem), 0ll);
    k2addr::clearForTest();
    // a null engine fn makes the call refuse
    g_sv.getModel = nullptr; g_sv.addModel = nullptr; g_sv.removeModel = nullptr;
    KT_CHECK(getModel(vp, 0) == nullptr && addModel(vp, "x") == nullptr && !removeModel(vp, 0));
    installFakes();
}

static void testCamera() {
    k2addr::clearForTest();
    uintptr_t view = arenaAlloc(0x400), cam = arenaAlloc(0x40);
    void* vp = reinterpret_cast<void*>(view);
    at<uint32_t>(cam, 0) = 0x0098C45C; at<uint32_t>(view, 0x78) = (uint32_t)cam;
    uint32_t c = 7, cv = 7;
    KT_CHECK(camera(vp, &c, &cv)); KT_CHECK_EQ(U(c), U(cam)); KT_CHECK_EQ(U(cv), 0x98C45Cull);
    KT_CHECK(camera(vp, nullptr, nullptr));
    at<uint32_t>(cam, 0) = 0x12345678;
    KT_CHECK(!camera(vp, &c, &cv)); KT_CHECK_EQ(U(c), U(cam)); KT_CHECK_EQ(U(cv), 0x12345678ull);
    at<uint32_t>(view, 0x78) = 0; c = cv = 7;
    KT_CHECK(!camera(vp, &c, &cv)); KT_CHECK_EQ(U(c), 0ull); KT_CHECK_EQ(U(cv), 0ull);
    at<uint32_t>(view, 0x78) = 0x20; c = cv = 7;                    // unreadable camera object: cam kept, vtable 0
    KT_CHECK(!camera(vp, &c, &cv)); KT_CHECK_EQ(U(c), 0x20ull); KT_CHECK_EQ(U(cv), 0ull);
    KT_CHECK(!camera(nullptr, &c, &cv) && c == 0 && cv == 0);
    at<uint32_t>(view, 0x78) = (uint32_t)cam; at<uint32_t>(cam, 0) = 0x98C45C;
    k2addr::setForTest(nullptr); KT_CHECK(!camera(vp, &c, &cv)); KT_CHECK_EQ(U(c), 0ull);
    k2addr::setForTest(&kZero); KT_CHECK(!camera(vp, &c, &cv));
    K2AddrTable noVt = kSteam; noVt.SceneCameraVtable = 0; k2addr::setForTest(&noVt);
    KT_CHECK(!camera(vp, &c, &cv)); KT_CHECK_EQ(U(cv), 0x98C45Cull);   // still reads, but never "matches" a zero expectation
    k2addr::clearForTest();
}

static void testCameraCalls() {
    k2addr::clearForTest(); installFakes(); resetCounters();
    uintptr_t view = arenaAlloc(0x400), cam = arenaAlloc(0x40), cvt = arenaAlloc(0x100);
    void* vp = reinterpret_cast<void*>(view);
    at<uint32_t>(cam, 0) = 0x0098C45C; at<uint32_t>(view, 0x78) = (uint32_t)cam;
    at<uint32_t>(cvt, 0x74) = 0xA1A1A1A1; at<uint32_t>(cvt, 0x44) = 0xB2B2B2B2;
    // happy path needs the camera vtable at the real address; point the table's expectation at our arena vtable instead
    K2AddrTable t = kSteam; t.SceneCameraVtable = cvt; k2addr::setForTest(&t);
    at<uint32_t>(cam, 0) = (uint32_t)cvt;
    char model[4]; 
    KT_CHECK(cameraAttach(vp, model, "camerahook", 1));
    KT_CHECK_EQ(U(nCamAttach), 1ull); KT_CHECK_EQ(U(caFn), 0xA1A1A1A1ull); KT_CHECK_EQ(U(caCam), U(cam)); KT_CHECK(caModel == model);
    KT_CHECK(strcmp(caNode, "camerahook") == 0); KT_CHECK_EQ((long long)caFlag, 1ll);
    KT_CHECK(cameraFov(vp, 42.5f));
    KT_CHECK_EQ(U(nCamFov), 1ull); KT_CHECK_EQ(U(cfFn), 0xB2B2B2B2ull); KT_CHECK_EQ(U(cfCam), U(cam)); KT_CHECK(cfFov == 42.5f);
    // null table
    k2addr::setForTest(nullptr); resetCounters();
    KT_CHECK(!cameraAttach(vp, model, "n", 1) && !cameraFov(vp, 1.f)); KT_CHECK_EQ(U(nCamAttach + nCamFov), 0ull);
    k2addr::setForTest(&kZero);
    KT_CHECK(!cameraAttach(vp, model, "n", 1) && !cameraFov(vp, 1.f)); KT_CHECK_EQ(U(nCamAttach + nCamFov), 0ull);
    // wrong camera vtable
    k2addr::setForTest(&t); at<uint32_t>(cam, 0) = 0x12345678;
    KT_CHECK(!cameraAttach(vp, model, "n", 1) && !cameraFov(vp, 1.f)); KT_CHECK_EQ(U(nCamAttach + nCamFov), 0ull);
    at<uint32_t>(cam, 0) = (uint32_t)cvt;
    KT_CHECK(!cameraAttach(nullptr, model, "n", 1) && !cameraFov(nullptr, 1.f));
    // zero offsets
    K2AddrTable z1 = t; z1.SceneCamVt_Attach = 0; k2addr::setForTest(&z1);
    KT_CHECK(!cameraAttach(vp, model, "n", 1)); KT_CHECK(cameraFov(vp, 1.f)); KT_CHECK_EQ(U(nCamAttach), 0ull);
    K2AddrTable z2 = t; z2.SceneCamVt_Fov = 0; k2addr::setForTest(&z2); resetCounters();
    KT_CHECK(!cameraFov(vp, 1.f)); KT_CHECK(cameraAttach(vp, model, "n", 1)); KT_CHECK_EQ(U(nCamFov), 0ull);
    // zero slot value
    at<uint32_t>(cvt, 0x74) = 0; resetCounters(); k2addr::setForTest(&t);
    KT_CHECK(!cameraAttach(vp, model, "n", 1)); KT_CHECK_EQ(U(nCamAttach), 0ull);
    // no engine fn
    at<uint32_t>(cvt, 0x74) = 0xA1A1A1A1; g_sv.callCamAttach = nullptr; g_sv.callCamFov = nullptr;
    KT_CHECK(!cameraAttach(vp, model, "n", 1) && !cameraFov(vp, 1.f));
    installFakes();
    k2addr::clearForTest();
}

static void testGobCheck() {
    k2addr::clearForTest(); installFakes();
    uintptr_t vt = makeVtable(); uintptr_t gob = makeGob(vt); void* gp = reinterpret_cast<void*>(gob);
    uint32_t vo = 5; GobSlotCheck ch[GobSlotCount]; memset(ch, 0, sizeof ch); int nBad = 9;
    KT_CHECK_EQ((long long)gobCheck(gp, &vo, ch, &nBad), (long long)GF_None);
    KT_CHECK_EQ(U(vo), U(vt)); KT_CHECK_EQ((long long)nBad, 0ll);
    bool ok = true;
    for (int i = 0; i < GobSlotCount; ++i) ok &= ch[i].off == kSlotOff[i] && ch[i].expect == kSlotVal[i] && ch[i].got == kSlotVal[i] && ch[i].name && ch[i].name[0];
    KT_CHECK(ok);
    KT_CHECK(strcmp(ch[0].name, "SetPosition") == 0 && strcmp(ch[4].name, "GetParent") == 0);
    KT_CHECK_EQ((long long)gobCheck(gp), (long long)GF_None);          // all out-params optional

    for (int i = 0; i < GobSlotCount; ++i) {
        uintptr_t bvt = makeVtable(i, 0xDEAD0000u + i); uintptr_t bg = makeGob(bvt);
        memset(ch, 0, sizeof ch); nBad = 0;
        KT_CHECK_EQ((long long)gobCheck(reinterpret_cast<void*>(bg), &vo, ch, &nBad), (long long)GF_Slot);
        KT_CHECK_EQ((long long)nBad, 1ll); KT_CHECK_EQ(U(ch[i].got), 0xDEAD0000ull + i); KT_CHECK_EQ(U(ch[i].expect), U(kSlotVal[i]));
        int others = 0; for (int j = 0; j < GobSlotCount; ++j) if (j != i && ch[j].got == ch[j].expect) ++others;
        KT_CHECK_EQ((long long)others, (long long)(GobSlotCount - 1));
    }
    // two wrong slots
    { uintptr_t v2 = makeVtable(0, 1); at<uint32_t>(v2, 0x108) = 2; uintptr_t g2 = makeGob(v2); nBad = 0;
      KT_CHECK_EQ((long long)gobCheck(reinterpret_cast<void*>(g2), nullptr, nullptr, &nBad), (long long)GF_Slot); KT_CHECK_EQ((long long)nBad, 2ll); }

    vo = 5; nBad = 5;
    KT_CHECK_EQ((long long)gobCheck(nullptr, &vo, ch, &nBad), (long long)GF_Unreadable); KT_CHECK_EQ(U(vo), 0ull); KT_CHECK_EQ((long long)nBad, 0ll);
    KT_CHECK_EQ((long long)gobCheck(reinterpret_cast<void*>(0x20), &vo), (long long)GF_Unreadable);   // non-null but unreadable
    uintptr_t g0 = makeGob(0);                                          // vtable 0
    vo = 5; KT_CHECK_EQ((long long)gobCheck(reinterpret_cast<void*>(g0), &vo), (long long)GF_VtUnreadable); KT_CHECK_EQ(U(vo), 0ull);
    uintptr_t gl = makeGob(0x20);                                       // vtable low (unreadable)
    vo = 0; KT_CHECK_EQ((long long)gobCheck(reinterpret_cast<void*>(gl), &vo), (long long)GF_VtUnreadable); KT_CHECK_EQ(U(vo), 0x20ull);

    k2addr::setForTest(nullptr); vo = 5; nBad = 5;
    KT_CHECK_EQ((long long)gobCheck(gp, &vo, nullptr, &nBad), (long long)GF_Unknown); KT_CHECK_EQ(U(vo), 0ull); KT_CHECK_EQ((long long)nBad, 0ll);
    k2addr::setForTest(&kZero); KT_CHECK_EQ((long long)gobCheck(gp), (long long)GF_Unknown);
    K2AddrTable t1 = kSteam; t1.GobVt_GetParent = 0; k2addr::setForTest(&t1); KT_CHECK_EQ((long long)gobCheck(gp), (long long)GF_Unknown);
    K2AddrTable t2 = kSteam; t2.GobGetPosition = 0; k2addr::setForTest(&t2); KT_CHECK_EQ((long long)gobCheck(gp), (long long)GF_Unknown);
    K2AddrTable t3 = kSteam; t3.GobReadable = 0; k2addr::setForTest(&t3); KT_CHECK_EQ((long long)gobCheck(gp), (long long)GF_Unknown);
    K2AddrTable t4 = kSteam; t4.GobVtReadable = 0; k2addr::setForTest(&t4); KT_CHECK_EQ((long long)gobCheck(gp), (long long)GF_Unknown);
    k2addr::clearForTest();
}

static void testGobPose() {
    k2addr::clearForTest(); installFakes(); resetCounters();
    uintptr_t vt = makeVtable(); void* gp = reinterpret_cast<void*>(makeGob(vt));
    float pos[3], q[4];
    KT_CHECK(gobGetPose(gp, pos, q));
    KT_CHECK_EQ((long long)nGetVec, 2ll);
    KT_CHECK_EQ(U(gvFn[0]), 0x459710ull); KT_CHECK_EQ(U(gvFn[1]), 0x459740ull); KT_CHECK(gvGob[0] == gp && gvGob[1] == gp);
    KT_CHECK(pos[0] == 1.5f && pos[1] == 2.5f && pos[2] == 3.5f);
    KT_CHECK(q[0] == 0.5f && q[1] == 0.25f && q[2] == 0.125f && q[3] == 0.0625f);

    // slot fns zero: defaults, no call
    resetCounters();
    at<uint32_t>(vt, 0x64) = 0;
    pos[0] = pos[1] = pos[2] = 9; q[0] = q[1] = q[2] = q[3] = 9;
    KT_CHECK(!gobGetPose(gp, pos, q));
    KT_CHECK(pos[0] == 0 && pos[1] == 0 && pos[2] == 0 && q[0] == 1 && q[1] == 0 && q[2] == 0 && q[3] == 0); KT_CHECK_EQ((long long)nGetVec, 0ll);
    at<uint32_t>(vt, 0x64) = 0x459710; at<uint32_t>(vt, 0x68) = 0;
    KT_CHECK(!gobGetPose(gp, pos, q)); KT_CHECK(q[0] == 1 && pos[0] == 0); KT_CHECK_EQ((long long)nGetVec, 0ll);
    at<uint32_t>(vt, 0x68) = 0x459740;
    KT_CHECK(!gobGetPose(nullptr, pos, q) && q[0] == 1);
    KT_CHECK(!gobGetPose(reinterpret_cast<void*>(0x20), pos, q));
    uintptr_t vtLow = makeGob(0x20);                                    // unreadable vtable
    KT_CHECK(!gobGetPose(reinterpret_cast<void*>(vtLow), pos, q) && q[0] == 1);
    KT_CHECK_EQ((long long)nGetVec, 0ll);
    k2addr::setForTest(nullptr); KT_CHECK(!gobGetPose(gp, pos, q)); KT_CHECK_EQ((long long)nGetVec, 0ll); k2addr::clearForTest();

    // setters
    KT_CHECK(gobSetPosition(gp, 1.f, 2.f, 3.f));
    KT_CHECK_EQ(U(spFn), 0x461490ull); KT_CHECK(spGob == gp && spV[0] == 1.f && spV[1] == 2.f && spV[2] == 3.f);
    KT_CHECK(gobSetOrientation(gp, 1.f, 0.f, 0.5f, -1.f));
    KT_CHECK_EQ(U(soFn), 0x4614E0ull); KT_CHECK(soGob == gp && soV[0] == 1.f && soV[1] == 0.f && soV[2] == 0.5f && soV[3] == -1.f);
    void* par = reinterpret_cast<void*>(1);
    KT_CHECK(gobGetParent(gp, &par)); KT_CHECK(par == gpResult); KT_CHECK_EQ(U(gpFn), 0x461530ull); KT_CHECK(gpGob == gp); KT_CHECK_EQ((long long)gpArg, -1ll);
    KT_CHECK(gobGetParent(gp, &par, 0) && gpArg == 0);
    int s = nSetPos, o = nSetOri, p = nGetParent;
    at<uint32_t>(vt, 0x5C) = 0; at<uint32_t>(vt, 0x60) = 0; at<uint32_t>(vt, 0x108) = 0;
    KT_CHECK(!gobSetPosition(gp, 0, 0, 0)); KT_CHECK(!gobSetOrientation(gp, 1, 0, 0, 0));
    par = reinterpret_cast<void*>(1); KT_CHECK(!gobGetParent(gp, &par)); KT_CHECK(par == nullptr);
    KT_CHECK(!gobSetPosition(nullptr, 0, 0, 0) && !gobSetOrientation(nullptr, 1, 0, 0, 0));
    par = reinterpret_cast<void*>(1); KT_CHECK(!gobGetParent(nullptr, &par) && par == nullptr);
    KT_CHECK(!gobSetPosition(reinterpret_cast<void*>(0x20), 0, 0, 0));
    KT_CHECK(s == nSetPos && o == nSetOri && p == nGetParent);
    // null callGetParent / callSetPos
    for (int i = 0; i < GobSlotCount; ++i) at<uint32_t>(vt, kSlotOff[i]) = kSlotVal[i];
    auto cp = g_sv.callGetParent; auto cs = g_sv.callSetPos; auto co = g_sv.callSetOri; auto cg = g_sv.callGetVec;
    g_sv.callGetParent = nullptr; g_sv.callSetPos = nullptr; g_sv.callSetOri = nullptr; g_sv.callGetVec = nullptr;
    KT_CHECK(!gobGetParent(gp, &par) && !gobSetPosition(gp, 0, 0, 0) && !gobSetOrientation(gp, 1, 0, 0, 0) && !gobGetPose(gp, pos, q));
    g_sv.callGetParent = cp; g_sv.callSetPos = cs; g_sv.callSetOri = co; g_sv.callGetVec = cg;
    // unfilled table
    k2addr::setForTest(&kZero);
    KT_CHECK(!gobSetPosition(gp, 0, 0, 0) && !gobSetOrientation(gp, 1, 0, 0, 0) && !gobGetParent(gp, &par) && !gobGetPose(gp, pos, q));
    KT_CHECK(s == nSetPos && o == nSetOri);
    k2addr::clearForTest();
}

static void testScale() {
    k2addr::clearForTest();
    uintptr_t gob = arenaAlloc(0x300); void* gp = reinterpret_cast<void*>(gob);
    at<float>(gob, 0x1e0) = 1.25f; float s = 0;
    KT_CHECK(gobGetScale(gp, &s)); KT_CHECK(s == 1.25f);
    KT_CHECK(gobSetScale(gp, 0.5f)); KT_CHECK(at<float>(gob, 0x1e0) == 0.5f);
    KT_CHECK(gobGetScale(gp, &s) && s == 0.5f);
    KT_CHECK(at<float>(gob, 0x1dc) == 0.f && at<float>(gob, 0x1e4) == 0.f);   // neighbours untouched
    KT_CHECK(!gobGetScale(nullptr, &s) && !gobSetScale(nullptr, 1.f));
    KT_CHECK(!gobGetScale(reinterpret_cast<void*>(0x20), &s) && !gobSetScale(reinterpret_cast<void*>(0x20), 1.f));
    K2AddrTable none = kSteam; none.Gob_Scale = 0; k2addr::setForTest(&none);
    KT_CHECK(!gobGetScale(gp, &s) && !gobSetScale(gp, 2.f)); KT_CHECK(at<float>(gob, 0x1e0) == 0.5f);
    k2addr::setForTest(nullptr); KT_CHECK(!gobGetScale(gp, &s) && !gobSetScale(gp, 2.f));
    k2addr::clearForTest();
}

// SH-3b-06 (WI-99): installFakes() replaces every call lambda, so the default thiscall trampolines of Engine never ran on host.
// A fresh Engine keeps them. Its `fn` slot is a 32-bit engine address, but a PIE test binary lives above 4 GB, so each host stub is
// reached through a 12-byte thunk (mov rax, imm64 ; jmp rax) in a MAP_32BIT page. On the x86-64 host __thiscall is ignored, so what
// is pinned is the argument order/count, the pass-through of the sentinel pointers, the return value and the size of the hidden
// out buffers (the stubs fill them exactly like the engine: 3 floats for a position, 4 for an orientation; ASan flags a short one).
static uint8_t* g_thunkPage = nullptr; static size_t g_thunkUsed = 0;
static uint32_t lowThunk(const void* target) {
    if (!g_thunkPage) {
        void* p = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
        if (p == MAP_FAILED) return 0;
        g_thunkPage = static_cast<uint8_t*>(p);
    }
    uint8_t* t = g_thunkPage + g_thunkUsed; g_thunkUsed += 16;
    uint64_t dst = reinterpret_cast<uint64_t>(target);
    t[0] = 0x48; t[1] = 0xB8; memcpy(t + 2, &dst, 8); t[10] = 0xFF; t[11] = 0xE0;   // mov rax, dst ; jmp rax
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(t));
}
static int dN; static void* dThis; static void* dArg1; static float dF[4]; static const char* dStr; static int dInt;
static void dReset() { dN = 0; dThis = dArg1 = nullptr; memset(dF, 0, sizeof dF); dStr = nullptr; dInt = 0; }
static void dSceneInit(void* sc, const char* room, float* pos, float* quat) { ++dN; dThis = sc; dStr = room; dArg1 = pos; memcpy(dF, quat, sizeof(float) * 4); }
static void* dSetPos(void* g, float* out, float x, float y, float z) { ++dN; dThis = g; dArg1 = out; out[0] = x; out[1] = y; out[2] = z; dF[0] = x; dF[1] = y; dF[2] = z; return out; }
static void* dSetOri(void* g, float* out, float w, float x, float y, float z) { ++dN; dThis = g; dArg1 = out; out[0] = w; out[1] = x; out[2] = y; out[3] = z; dF[0] = w; dF[1] = x; dF[2] = y; dF[3] = z; return out; }
static void* dGetVec(void* g, float* out) { ++dN; dThis = g; dArg1 = out; out[0] = 4.f; out[1] = 5.f; out[2] = 6.f; out[3] = 7.f; return out; }
static void* dGetParent(void* g, int arg) { ++dN; dThis = g; dInt = arg; return reinterpret_cast<void*>(0x7777); }
static void dCamAttach(void* cam, void* model, const char* node, int flag) { ++dN; dThis = cam; dArg1 = model; dStr = node; dInt = flag; }
static void dCamFov(void* cam, float fov) { ++dN; dThis = cam; dF[0] = fov; }
static void testDefaultTrampolines() {
    k2addr::clearForTest();
    Engine e;   // default trampolines, not the installFakes overrides
    void* const g = reinterpret_cast<void*>(0x5A5A0010); void* const x1 = reinterpret_cast<void*>(0x5A5A0020);
    const uint32_t tSi = lowThunk(reinterpret_cast<void*>(&dSceneInit)), tPos = lowThunk(reinterpret_cast<void*>(&dSetPos)),
                   tOri = lowThunk(reinterpret_cast<void*>(&dSetOri)), tGv = lowThunk(reinterpret_cast<void*>(&dGetVec)),
                   tGp = lowThunk(reinterpret_cast<void*>(&dGetParent)), tCa = lowThunk(reinterpret_cast<void*>(&dCamAttach)),
                   tCf = lowThunk(reinterpret_cast<void*>(&dCamFov));
    KT_CHECK(tSi && tPos && tOri && tGv && tGp && tCa && tCf);
    if (!(tSi && tPos && tOri && tGv && tGp && tCa && tCf)) return;

    dReset(); float pos[3] = { 1.f, 2.f, 3.f }, quat[4] = { 0.f, 0.5f, 0.25f, 1.f };
    e.callSceneInit(tSi, reinterpret_cast<uintptr_t>(x1), "room_x", pos, quat);
    KT_CHECK_EQ((long long)dN, 1ll); KT_CHECK(dThis == x1 && dStr && strcmp(dStr, "room_x") == 0 && dArg1 == pos);
    KT_CHECK(dF[0] == 0.f && dF[1] == 0.5f && dF[2] == 0.25f && dF[3] == 1.f);

    dReset();
    e.callSetPos(tPos, g, 1.5f, -2.25f, 3.0f);   // distinct values expose a swapped axis
    KT_CHECK_EQ((long long)dN, 1ll); KT_CHECK(dThis == g && dArg1 != nullptr);
    KT_CHECK(dF[0] == 1.5f && dF[1] == -2.25f && dF[2] == 3.0f);

    dReset();
    e.callSetOri(tOri, g, 0.5f, 0.25f, -0.75f, 1.0f);
    KT_CHECK_EQ((long long)dN, 1ll); KT_CHECK(dThis == g && dArg1 != nullptr);
    KT_CHECK(dF[0] == 0.5f && dF[1] == 0.25f && dF[2] == -0.75f && dF[3] == 1.0f);

    dReset(); float out[4] = { 0, 0, 0, 0 };
    e.callGetVec(tGv, g, out);   // the caller's buffer is passed straight through (up to 4 floats)
    KT_CHECK_EQ((long long)dN, 1ll); KT_CHECK(dThis == g && dArg1 == out);
    KT_CHECK(out[0] == 4.f && out[1] == 5.f && out[2] == 6.f && out[3] == 7.f);

    dReset();
    KT_CHECK(e.callGetParent(tGp, g, 9) == reinterpret_cast<void*>(0x7777));
    KT_CHECK_EQ((long long)dN, 1ll); KT_CHECK(dThis == g); KT_CHECK_EQ((long long)dInt, 9ll);

    dReset();
    e.callCamAttach(tCa, reinterpret_cast<uintptr_t>(x1), g, "camerahook", 3);
    KT_CHECK_EQ((long long)dN, 1ll); KT_CHECK(dThis == x1 && dArg1 == g && dStr && strcmp(dStr, "camerahook") == 0); KT_CHECK_EQ((long long)dInt, 3ll);

    dReset();
    e.callCamFov(tCf, reinterpret_cast<uintptr_t>(x1), 62.5f);
    KT_CHECK_EQ((long long)dN, 1ll); KT_CHECK(dThis == x1 && dF[0] == 62.5f);
}

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    testRebind();
    testGate();
    testCreateView();
    testInitScene();
    testModels();
    testCamera();
    testCameraCalls();
    testGobCheck();
    testGobPose();
    testScale();
    testDefaultTrampolines();
    return kt::Report("sceneview_test");
}
