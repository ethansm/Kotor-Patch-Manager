// common_contract_test.cpp -- Patches/Common CONTRACT test for the Inventory 3D Viewport (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/{saferead,sceneview,k2addr}_test.cpp). Every message starts with
// "inv3d relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to
// SafeRead.h / SceneView.h / K2Addr.h / GameAddr.h re-runs this test: a Common change that would break the viewer fails here first.
//
// Areas: (1) gameaddr / kSteam constants (compile time), (2) saferead, (3) sceneview on a fake engine in a MAP_32BIT arena
// (every engine call is a fake in sceneview::g_sv), (4) unknown exe (k2addr table null): nothing is called. Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SafeRead.h"
#include "GameAddr.h"
#include "K2Addr.h"
#include "SceneView.h"
#include "check.h"
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HCHECK(why, expr) ::kt::Check("inv3d relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("inv3d relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))
#define U(x) static_cast<unsigned long long>(x)

// ============================================================ 1. GameAddr / kSteam (compile time)
// The first group is logged / used directly by the mod; the second is the original inv3d literal set now held in the K2Addr table.
#define HADDR(name, val) static_assert(gameaddr::name == (val), "inv3d relies on: gameaddr::" #name " == " #val " (validated Steam address)")
HADDR(InitControl, 0x0040F620); HADDR(ExoCtor, 0x00733570); HADDR(ExoDtor, 0x00733780); HADDR(New, 0x00919723);
HADDR(LabelCtor, 0x00419740); HADDR(LabelVtable, 0x009878BC); HADDR(ScreenW, 0x009F42A4); HADDR(ScreenH, 0x009F42A8);
HADDR(SceneCameraVtable, 0x0098C45C); HADDR(GobVtable, 0x0098B5CC);
#define KADDR(f, val) static_assert(kSteam.f == (val), "inv3d relies on: kSteam." #f " == " #val " (original inv3d literal)")
KADDR(GuiGateGlobal, 0x009F42B4); KADDR(GuiControlCtor, 0x00418990); KADDR(GuiSceneCtor, 0x00417F40);
KADDR(SceneAddModel, 0x00418360); KADDR(SceneRemoveModel, 0x00418430); KADDR(SceneGetModel, 0x004184B0);
KADDR(SceneViewVtable, 0x009A3D34); KADDR(SceneCameraVtable, 0x0098C45C); KADDR(GobVtable, 0x0098B5CC);
KADDR(GobSetPosition, 0x00461490); KADDR(GobSetOrientation, 0x004614E0); KADDR(GobGetPosition, 0x00459710);
KADDR(GobGetOrientation, 0x00459740); KADDR(GobGetParent, 0x00461530);
KADDR(SceneViewSize, 0x400); KADDR(View_Scene, 0x60); KADDR(View_SceneObj, 0x74); KADDR(View_Camera, 0x78); KADDR(View_ModelCount, 0x80);
KADDR(SceneVt_Init, 0x70); KADDR(GobVt_SetPosition, 0x5C); KADDR(GobVt_SetOrientation, 0x60); KADDR(GobVt_GetPosition, 0x64);
KADDR(GobVt_GetOrientation, 0x68); KADDR(GobVt_GetParent, 0x108); KADDR(GobVtReadable, 0x10c); KADDR(GobReadable, 0x200); KADDR(Gob_Scale, 0x1e0);
static_assert(sceneview::GobSlotCount == 5, "inv3d relies on: gobCheck reports exactly 5 slots (the mod sizes its GobSlotCheck array with GobSlotCount)");
static_assert(sceneview::GF_None == 0 && sceneview::SF_None == 0, "inv3d relies on: GF_None / SF_None == 0");

// ============================================================ arena + fakes
using namespace sceneview;
static uint8_t* g_arena = nullptr; static size_t g_used = 0; constexpr size_t ArenaSize = 1u << 20;
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { printf("arena full\n"); exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used); memset(g_arena + g_used, 0, n); g_used += n; return a;
}
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
static const K2AddrTable kZero{};

static int nAlloc, nCtl, nScene, nInit, nExoCtor, nExoDtor, nAdd, nRem, nGet, nSceneInit, nSetPos, nSetOri, nGetVec, nGetParent;
static unsigned lastAllocSize; static bool allocFails;
static void* lastCtlThis; static void* lastSceneThis; static uint32_t vtAtCtlCtor;
static void* lastInitPanel; static void* lastInitView; static char lastInitTag[16]; static int lastInitA, lastInitB;
static void* lastAddThis; static char lastAddRes[16]; static int lastAddIdx;
static void* lastRemThis; static int lastRemIdx, lastRemDel;
static void* lastGetThis; static int lastGetIdx;
static uint32_t siFn; static uintptr_t siScene; static char siRoom[32];
static uint32_t spFn, soFn; static void* spGob; static void* soGob; static float spV[3], soV[4];
static uint32_t gvFn[2]; static int gpArg; static uint32_t gpFn; static void* gpResult = reinterpret_cast<void*>(0x7003);

static void* fAlloc(unsigned n) { ++nAlloc; lastAllocSize = n; if (allocFails) return nullptr; uintptr_t a = arenaAlloc(n); memset(reinterpret_cast<void*>(a), 0xAB, n); return reinterpret_cast<void*>(a); }
static void fCtl(void* t) { ++nCtl; lastCtlThis = t; vtAtCtlCtor = *static_cast<uint32_t*>(t); }
static void fScene(void* t) { ++nScene; lastSceneThis = t; }
static void fInitCtl(void* panel, void* view, void* exo, int a, int b) { ++nInit; lastInitPanel = panel; lastInitView = view; strncpy(lastInitTag, static_cast<char*>(exo), 15); lastInitA = a; lastInitB = b; }
static void* fExoCtor(void* e, const char* s) { ++nExoCtor; memset(e, 0, 16); strncpy(static_cast<char*>(e), s, 15); return e; }
static void fExoDtor(void*) { ++nExoDtor; }
static void* fAdd(void* t, void* exo, int idx) { ++nAdd; lastAddThis = t; strncpy(lastAddRes, static_cast<char*>(exo), 15); lastAddIdx = idx; return reinterpret_cast<void*>(0x7001); }
static void fRem(void* t, int idx, int del) { ++nRem; lastRemThis = t; lastRemIdx = idx; lastRemDel = del; }
static void* fGet(void* t, int idx) { ++nGet; lastGetThis = t; lastGetIdx = idx; return reinterpret_cast<void*>(0x7002); }
static void fSceneInit(uint32_t fn, uintptr_t sc, const char* room, float*, float*) { ++nSceneInit; siFn = fn; siScene = sc; strncpy(siRoom, room, 31); }
static void fSetPos(uint32_t fn, void* g, float x, float y, float z) { ++nSetPos; spFn = fn; spGob = g; spV[0] = x; spV[1] = y; spV[2] = z; }
static void fSetOri(uint32_t fn, void* g, float w, float x, float y, float z) { ++nSetOri; soFn = fn; soGob = g; soV[0] = w; soV[1] = x; soV[2] = y; soV[3] = z; }
static void fGetVec(uint32_t fn, void*, float* out) {
    gvFn[nGetVec & 1] = fn;
    if (nGetVec == 0) { out[0] = 1.5f; out[1] = 2.5f; out[2] = 3.5f; } else { out[0] = 0.5f; out[1] = 0.25f; out[2] = 0.125f; out[3] = 0.0625f; }
    ++nGetVec;
}
static void* fGetParent(uint32_t fn, void*, int arg) { ++nGetParent; gpFn = fn; gpArg = arg; return gpResult; }

static void resetCounters() {
    nAlloc = nCtl = nScene = nInit = nExoCtor = nExoDtor = nAdd = nRem = nGet = nSceneInit = nSetPos = nSetOri = nGetVec = nGetParent = 0;
    allocFails = false; lastAllocSize = 0; lastCtlThis = lastSceneThis = lastInitPanel = lastInitView = nullptr; lastInitTag[0] = 0; lastInitA = lastInitB = 0; vtAtCtlCtor = 0;
}
static void installFakes() {
    k2addr::clearForTest(); g_sv.rebind();
    g_sv.alloc = fAlloc; g_sv.controlCtor = fCtl; g_sv.sceneCtor = fScene; g_sv.initControl = fInitCtl; g_sv.exoCtor = fExoCtor; g_sv.exoDtor = fExoDtor;
    g_sv.addModel = fAdd; g_sv.removeModel = fRem; g_sv.getModel = fGet;
    g_sv.callSceneInit = fSceneInit; g_sv.callSetPos = fSetPos; g_sv.callSetOri = fSetOri; g_sv.callGetVec = fGetVec; g_sv.callGetParent = fGetParent;
}
static const uint32_t kSlotVal[GobSlotCount] = { 0x00461490, 0x004614E0, 0x00459710, 0x00459740, 0x00461530 };
static const int kSlotOff[GobSlotCount] = { 0x5C, 0x60, 0x64, 0x68, 0x108 };
static const char* const kSlotName[GobSlotCount] = { "SetPosition", "SetOrientation", "GetPosition", "GetOrientation", "GetParent" };
static uintptr_t makeVtable(int wrongIdx = -1, uint32_t wrongVal = 0) {
    uintptr_t vt = arenaAlloc(0x200);
    for (int i = 0; i < GobSlotCount; ++i) at<uint32_t>(vt, kSlotOff[i]) = (i == wrongIdx) ? wrongVal : kSlotVal[i];
    return vt;
}
static uintptr_t makeGob(uintptr_t vt) { uintptr_t g = arenaAlloc(0x300); at<uint32_t>(g, 0) = (uint32_t)vt; return g; }

// ============================================================ 2. SafeRead
static void saferead_contract() {
    uintptr_t buf = arenaAlloc(64);
    at<uint32_t>(buf, 8) = 0xCAFEF00D; at<float>(buf, 16) = 0.5f;
    saferead::beginScope();
    uint32_t v = 0x11111111;
    HCHECK("readAt on readable memory returns true", saferead::readAt(1u, buf, 8, &v));
    HEQ("readAt delivers the value", U(v), 0xCAFEF00Dull);
    float f = 1.f; HCHECK("readAt<float> works (item/scale reads)", saferead::readAt(1u, buf, 16, &f) && f == 0.5f);
    v = 0x22222222;
    HCHECK("readAt below 0x10000 returns false", !saferead::readAt(1u, 0x20, 0, &v));
    HEQ("readAt failure leaves *out untouched (low address; the mod pre-seeds defaults)", U(v), 0x22222222ull);
    HCHECK("readAt with base 0 returns false", !saferead::readAt(1u, 0, 0x30, &v));
    HEQ("readAt failure leaves *out untouched (base 0)", U(v), 0x22222222ull);
    HCHECK("probeWrite true on arena memory (VarGuard substitutes the item word)", saferead::probeWrite(1u, reinterpret_cast<void*>(buf + 24), 4));
    HCHECK("probeWrite false below 0x10000", !saferead::probeWrite(1u, reinterpret_cast<void*>(0x20), 4));
    HCHECK("probeRead true on arena memory", saferead::probeRead(1u, reinterpret_cast<void*>(buf), 4));
    bool ok = true; for (int i = 0; i < 5; ++i) { saferead::beginScope(); ok = ok && saferead::readAt(1u, buf, 8, &v); }
    HCHECK("beginScope is callable repeatedly (every hook entry) and reads still work", ok);
}

// ============================================================ 3. SceneView
static void sceneview_contract() {
    // ---- gateOpen: int at GuiGateGlobal ----
    installFakes();
    uintptr_t cell = arenaAlloc(16);
    K2AddrTable t = kSteam; t.GuiGateGlobal = cell;
    k2addr::setForTest(&t);
    int gate = 99;
    at<int>(cell, 0) = 0; HCHECK("gateOpen false while the gate int is 0", !gateOpen(&gate)); HEQ("gateOpen reports 0", (long long)gate, 0ll);
    at<int>(cell, 0) = 7; HCHECK("gateOpen true when the gate int is non-zero", gateOpen(&gate)); HEQ("gateOpen reports the int read (the mod logs it)", (long long)gate, 7ll);
    k2addr::clearForTest(); g_sv.rebind();

    // ---- createView ----
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
    HEQ("initScene null Scene* -> SF_SceneBad", (long long)initScene(vp, "gui3D_room", &so), (long long)SF_SceneBad);
    HEQ("initScene null Scene* reports *sceneOut == 0 (mod logs it)", U(so), 0ull);
    at<uint32_t>(view, 0x74) = (uint32_t)scene; at<uint32_t>(scene, 0) = 0x20;
    HEQ("initScene unreadable scene vtable -> SF_VtableBad", (long long)initScene(vp, "gui3D_room"), (long long)SF_VtableBad);
    HEQ("initScene refusals make no engine call", (long long)nSceneInit, 1ll);
    at<uint32_t>(scene, 0) = (uint32_t)svt;

    // ---- model calls / count ----
    at<int>(view, 0x80) = 3;
    HEQ("modelCount reads view+0x80", (long long)modelCount(vp), 3ll);
    HEQ("modelCount of a null view is -1", (long long)modelCount(nullptr), -1ll);
    HCHECK("getModel passes view+0x60 and the index", getModel(vp, 1) == reinterpret_cast<void*>(0x7002) && lastGetThis == reinterpret_cast<void*>(view + 0x60) && lastGetIdx == 1);
    HCHECK("getModel(null view) is null with no call", getModel(nullptr, 0) == nullptr && nGet == 1);
    HCHECK("addModel(view, rig, -1) passes view+0x60, resref, idx -1", addModel(vp, "p_rig", -1) == reinterpret_cast<void*>(0x7001) &&
           lastAddThis == reinterpret_cast<void*>(view + 0x60) && !strcmp(lastAddRes, "p_rig") && lastAddIdx == -1);
    HCHECK("addModel balances the CExoString ctor/dtor", nExoCtor == nExoDtor && nExoCtor == 1);
    HCHECK("removeModel(view, 1, true) passes view+0x60, idx, deleteIt=1", removeModel(vp, 1, true) && lastRemThis == reinterpret_cast<void*>(view + 0x60) && lastRemIdx == 1 && lastRemDel == 1);

    // ---- camera ----
    uintptr_t cam = arenaAlloc(0x20); at<uint32_t>(cam, 0) = 0x98C45C; at<uint32_t>(view, 0x78) = (uint32_t)cam;
    uint32_t c = 0, cv = 0;
    HCHECK("camera true for the 0x98C45C vtable", camera(vp, &c, &cv)); HCHECK("camera fills cam and camVt", c == cam && cv == 0x98C45C);
    at<uint32_t>(cam, 0) = 0x1234; c = cv = 0;
    HCHECK("camera false for a foreign vtable but still fills cam/camVt (mod logs both)", !camera(vp, &c, &cv) && c == cam && cv == 0x1234);

    // ---- gobCheck ----
    installFakes(); resetCounters();
    uintptr_t vt = makeVtable(), gob = makeGob(vt); void* gp = reinterpret_cast<void*>(gob);
    uint32_t vtOut = 0; GobSlotCheck ck[GobSlotCount]; int bad = -1;
    HEQ("gobCheck with all 5 slots right -> GF_None", (long long)gobCheck(gp, &vtOut, ck, &bad), (long long)GF_None);
    HCHECK("gobCheck GF_None reports vtable, 0 bad", vtOut == vt && bad == 0);
    uintptr_t vt2 = makeVtable(2, 0xDEAD0001u), gob2 = makeGob(vt2);
    HEQ("gobCheck with one wrong slot -> GF_Slot", (long long)gobCheck(reinterpret_cast<void*>(gob2), &vtOut, ck, &bad), (long long)GF_Slot);
    HEQ("gobCheck GF_Slot nBad == 1", (long long)bad, 1ll);
    HCHECK("gobCheck names the mismatching slot (off/name/expect/got, all four are logged)",
           ck[2].off == 0x64 && !strcmp(ck[2].name, kSlotName[2]) && ck[2].expect == 0x00459710 && ck[2].got == 0xDEAD0001u);
    bool others = true; for (int i = 0; i < GobSlotCount; ++i) if (i != 2 && (ck[i].got != ck[i].expect || ck[i].off != kSlotOff[i] || strcmp(ck[i].name, kSlotName[i]))) others = false;
    HCHECK("gobCheck fills the matching slots too (off/name/expect==got)", others);
    HEQ("gobCheck null gob -> GF_Unreadable", (long long)gobCheck(nullptr), (long long)GF_Unreadable);
    HEQ("gobCheck gob below 0x10000 -> GF_Unreadable", (long long)gobCheck(reinterpret_cast<void*>(0x20)), (long long)GF_Unreadable);
    uintptr_t gob3 = makeGob(0x20);
    HEQ("gobCheck vtable < 0x10000 -> GF_VtUnreadable", (long long)gobCheck(reinterpret_cast<void*>(gob3), &vtOut), (long long)GF_VtUnreadable);
    HEQ("gobCheck GF_VtUnreadable still reports the vtable read", U(vtOut), 0x20ull);

    // ---- gobGetPose ----
    float pos[3] = {9, 9, 9}, q[4] = {9, 9, 9, 9};
    HCHECK("gobGetPose fills pos/q through GetPosition(+0x64) then GetOrientation(+0x68)", gobGetPose(gp, pos, q) && nGetVec == 2 &&
           gvFn[0] == 0x00459710 && gvFn[1] == 0x00459740 && pos[0] == 1.5f && pos[2] == 3.5f && q[0] == 0.5f && q[3] == 0.0625f);
    float pos2[3] = {9, 9, 9}, q2[4] = {9, 9, 9, 9}; int before = nGetVec;
    HCHECK("gobGetPose of a null gob fails", !gobGetPose(nullptr, pos2, q2) && nGetVec == before);
    HCHECK("gobGetPose failure leaves pos=0 and q=(1,0,0,0) (mod logs them unguarded)", pos2[0] == 0 && pos2[1] == 0 && pos2[2] == 0 && q2[0] == 1 && q2[1] == 0 && q2[2] == 0 && q2[3] == 0);

    // ---- setters / parent / scale ----
    HCHECK("gobSetPosition calls slot +0x5C", gobSetPosition(gp, 1.f, 2.f, 3.f) && spFn == 0x00461490 && spGob == gp && spV[0] == 1.f && spV[1] == 2.f && spV[2] == 3.f);
    HCHECK("gobSetOrientation (w,x,y,z) calls slot +0x60", gobSetOrientation(gp, 1.f, 0.f, 0.5f, -1.f) && soFn == 0x004614E0 && soGob == gp && soV[0] == 1.f && soV[2] == 0.5f && soV[3] == -1.f);
    void* par = reinterpret_cast<void*>(1);
    HCHECK("gobGetParent default arg is -1 (the mod's explicit -1) via slot +0x108", gobGetParent(gp, &par, -1) && par == gpResult && gpArg == -1 && gpFn == 0x00461530);
    at<float>(gob, 0x1e0) = 1.25f; float s = 0;
    HCHECK("gobGetScale reads Gob+0x1e0", gobGetScale(gp, &s) && s == 1.25f);
    HCHECK("gobSetScale writes Gob+0x1e0 only", gobSetScale(gp, 0.5f) && at<float>(gob, 0x1e0) == 0.5f && at<float>(gob, 0x1dc) == 0.f && at<float>(gob, 0x1e4) == 0.f);
    HCHECK("gobGetScale/gobSetScale refuse an unreadable gob", !gobGetScale(reinterpret_cast<void*>(0x20), &s) && !gobSetScale(reinterpret_cast<void*>(0x20), 1.f));
}

// ============================================================ 4. unknown exe
static void unknown_exe_contract() {
    installFakes(); resetCounters();
    uintptr_t panel = arenaAlloc(0x100), vt = makeVtable(), gob = makeGob(vt);
    uintptr_t view = arenaAlloc(0x400); at<int>(view, 0x80) = 3;
    k2addr::setForTest(nullptr);
    HCHECK("unknown exe: createView returns null", createView(panel, "3D_MODEL") == nullptr);
    HEQ("unknown exe: modelCount is -1", (long long)modelCount(reinterpret_cast<void*>(view)), -1ll);
    HEQ("unknown exe: gobCheck is GF_Unknown", (long long)gobCheck(reinterpret_cast<void*>(gob)), (long long)GF_Unknown);
    HEQ("unknown exe: initScene is SF_Unknown", (long long)initScene(reinterpret_cast<void*>(view), "gui3D_room"), (long long)SF_Unknown);
    HCHECK("unknown exe: no engine call made", nAlloc + nCtl + nScene + nInit + nExoCtor + nExoDtor + nAdd + nRem + nGet + nSceneInit + nSetPos + nSetOri + nGetVec + nGetParent == 0);
    k2addr::clearForTest(); g_sv.rebind();
}

int main() {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    saferead_contract();
    sceneview_contract();
    unknown_exe_contract();
    return kt::Report("common_contract_test (inventory_3d_viewport)");
}
