// SceneView.h -- header-only helper for a mod-owned CSWGui3DSceneView block (KOTOR2 Patch Manager mods, i686).
// The block recipe and scene-model access that Mod 1 (Inventory3DViewport.cpp 0.2.0-dev 290-317, 597-650) and Mod 5
// (KPM Patches/EquipCharPreview/EquipCharPreview.cpp 308-312, 942-985; repo mirror 241-245, 874-917) carried as identical copies: allocate a view block, construct it as a
// CSWGui3DSceneView, bind it to a layout tag, initialise its scene room, add / get / remove models, read the camera, and
// guard + pose a scene model (Gob) through its vtable. Migration plan: project design doc 23.
//
// Engine facts (Steam Aspyr, verified in Mods 1/5): view +0x60 CSWGuiScene sub-object (the `this` of AddModel / GetModel /
// RemoveModel), +0x74 Scene*, +0x78 camera object (vtable 0x98C45C), +0x80 model count; Scene* vtable +0x70 = init(room, pos, quat).
// Gob vtable 0x98B5CC: +0x5C SetPosition, +0x60 SetOrientation(w,x,y,z), +0x64 GetPosition, +0x68 GetOrientation, +0x108 GetParent;
// Gob +0x1E0 = uniform scale (float).
// Per-version addresses/offsets come from the K2Addr.h table (k2addr::get()); a null table or a zero field makes every helper
// refuse (return 0 / -1 / false / a fail code), never call address 0. Every engine read goes through saferead (site 0x6D00).
// Host tests: every engine call (direct or through a vtable slot) is a function pointer in sceneview::Engine (g_sv), so a test
// substitutes fakes (see sceneview_test.cpp).
#pragma once
#include "SafeRead.h"
#include "K2Addr.h"
#include <string.h>

namespace sceneview {

using NewFn = void* (__cdecl*)(unsigned);
using CtorFn = void(__thiscall*)(void*);
using InitCtlFn = void(__thiscall*)(void*, void*, void*, int, int);
using ExoCtorFn = void*(__thiscall*)(void*, const char*);
using ExoDtorFn = void(__thiscall*)(void*);
using AddModelFn = void*(__thiscall*)(void*, void*, int);
using RemoveModelFn = void(__thiscall*)(void*, int, int);
using GetModelFn = void*(__thiscall*)(void*, int);

enum SceneFail { SF_None = 0, SF_Unknown, SF_SceneBad, SF_VtableBad };          // initScene result
enum GobFail { GF_None = 0, GF_Unknown, GF_Unreadable, GF_VtUnreadable, GF_Slot };   // gobCheck result
constexpr int GobSlotCount = 5;
struct GobSlotCheck { int off; uint32_t expect, got; const char* name; };

struct Engine {
    NewFn alloc = nullptr;
    CtorFn controlCtor = nullptr;
    CtorFn sceneCtor = nullptr;
    InitCtlFn initControl = nullptr;
    ExoCtorFn exoCtor = nullptr;
    ExoDtorFn exoDtor = nullptr;
    AddModelFn addModel = nullptr;
    RemoveModelFn removeModel = nullptr;
    GetModelFn getModel = nullptr;
    // Vtable-slot calls: `fn` is the slot value read from engine memory (a host test ignores it and records the call).
    void (*callSceneInit)(uint32_t fn, uintptr_t scene, const char* room, float* pos, float* quat) =
        [](uint32_t fn, uintptr_t scene, const char* room, float* pos, float* quat) {
            reinterpret_cast<void(__thiscall*)(void*, const char*, float*, float*)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(scene), room, pos, quat);
        };
    void (*callSetPos)(uint32_t fn, void* gob, float x, float y, float z) = [](uint32_t fn, void* gob, float x, float y, float z) {
        float out[3];
        reinterpret_cast<void*(__thiscall*)(void*, float*, float, float, float)>(static_cast<uintptr_t>(fn))(gob, out, x, y, z);
    };
    void (*callSetOri)(uint32_t fn, void* gob, float w, float x, float y, float z) = [](uint32_t fn, void* gob, float w, float x, float y, float z) {
        float out[4];
        reinterpret_cast<void*(__thiscall*)(void*, float*, float, float, float, float)>(static_cast<uintptr_t>(fn))(gob, out, w, x, y, z);
    };
    void (*callGetVec)(uint32_t fn, void* gob, float* out) = [](uint32_t fn, void* gob, float* out) {
        reinterpret_cast<void*(__thiscall*)(void*, float*)>(static_cast<uintptr_t>(fn))(gob, out);
    };
    void* (*callGetParent)(uint32_t fn, void* gob, int arg) = [](uint32_t fn, void* gob, int arg) {
        return reinterpret_cast<void*(__thiscall*)(void*, int)>(static_cast<uintptr_t>(fn))(gob, arg);
    };

    void (*callCamAttach)(uint32_t fn, uintptr_t cam, void* model, const char* node, int flag) =
        [](uint32_t fn, uintptr_t cam, void* model, const char* node, int flag) {
            reinterpret_cast<void(__thiscall*)(void*, void*, const char*, int)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(cam), model, node, flag);
        };
    void (*callCamFov)(uint32_t fn, uintptr_t cam, float fov) = [](uint32_t fn, uintptr_t cam, float fov) {
        reinterpret_cast<void(__thiscall*)(void*, float)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(cam), fov);
    };

    Engine() { rebind(); }
    // Re-point the engine-call fields at the current K2Addr.h table (also resets test fakes in those fields).
    void rebind() {
        using k2addr::fnptr;
        alloc = fnptr<NewFn>(&K2AddrTable::New);
        controlCtor = fnptr<CtorFn>(&K2AddrTable::GuiControlCtor);
        sceneCtor = fnptr<CtorFn>(&K2AddrTable::GuiSceneCtor);
        initControl = fnptr<InitCtlFn>(&K2AddrTable::InitControl);
        exoCtor = fnptr<ExoCtorFn>(&K2AddrTable::ExoCtor);
        exoDtor = fnptr<ExoDtorFn>(&K2AddrTable::ExoDtor);
        addModel = fnptr<AddModelFn>(&K2AddrTable::SceneAddModel);
        removeModel = fnptr<RemoveModelFn>(&K2AddrTable::SceneRemoveModel);
        getModel = fnptr<GetModelFn>(&K2AddrTable::SceneGetModel);
    }
};
inline Engine g_sv;   // substitutable by tests; defaults come from the K2Addr.h table for the running exe

namespace detail {
constexpr unsigned SiteBase = 0x6D00;
template <typename T> inline bool rd(uintptr_t base, int off, T* out) { return saferead::readAt<T>(SiteBase, base, off, out); }
inline int off(int K2AddrTable::* m) { return k2addr::off(m); }
inline uint32_t vtab(uintptr_t K2AddrTable::* m) { return static_cast<uint32_t>(k2addr::addr(m)); }
inline uintptr_t u(const void* p) { return reinterpret_cast<uintptr_t>(p); }
// The scene sub-object (the `this` of the model calls), or nullptr when the view or the offset is unknown.
inline void* scene(void* view) { int o = off(&K2AddrTable::View_Scene); return view && o ? static_cast<unsigned char*>(view) + o : nullptr; }
// Gob vtable slot value at byte offset `slotOff` (0 = unknown / unreadable).
inline uint32_t gobSlot(void* gob, int slotOff) {
    uint32_t vt = 0, fn = 0;
    if (!gob || !slotOff || !rd<uint32_t>(u(gob), 0, &vt) || !vt || !rd<uint32_t>(vt, slotOff, &fn)) return 0;
    return fn;
}
}

// ---- the view block ----
// GUI gate global (non-zero once the GUI/3D layer is up). *value = the int read (0 on failure). False = zero, unreadable or unknown.
inline bool gateOpen(int* value = nullptr) {
    int v = 0; uintptr_t a = k2addr::addr(&K2AddrTable::GuiGateGlobal);
    if (a) detail::rd<int>(a, 0, &v);
    if (value) *value = v;
    return v != 0;
}

// Allocates a zeroed view block (SceneViewSize bytes), constructs it as a CSWGui3DSceneView (control ctor, scene-view vtable,
// scene ctor) and binds it to the layout control `tag` of `panel` (InitControl addToList=1, scale=1: it is appended to the panel's
// control list). Valid only while the panel's layout is open (inside the panel ctor). Returns the view, or nullptr when
// panel/tag is null, a needed engine address or offset is unknown, or the allocation failed. Read the bound id / extent afterwards.
inline unsigned char* createView(uintptr_t panel, const char* tag) {
    using namespace detail;
    const int size = off(&K2AddrTable::SceneViewSize), oScene = off(&K2AddrTable::View_Scene);
    const uint32_t vtable = vtab(&K2AddrTable::SceneViewVtable);
    if (!panel || !tag || !g_sv.alloc || !g_sv.controlCtor || !g_sv.sceneCtor || !g_sv.initControl || !g_sv.exoCtor || !g_sv.exoDtor ||
        size <= 0 || !oScene || !vtable) return nullptr;
    unsigned char* view = static_cast<unsigned char*>(g_sv.alloc(static_cast<unsigned>(size)));
    if (!view) return nullptr;
    memset(view, 0, size);
    g_sv.controlCtor(view);
    *reinterpret_cast<uint32_t*>(view) = vtable;
    g_sv.sceneCtor(view + oScene);
    char exo[16] = {0};
    g_sv.exoCtor(exo, tag);
    g_sv.initControl(reinterpret_cast<void*>(panel), view, exo, 1, 1);
    g_sv.exoDtor(exo);
    return view;
}

// Initialises the view's Scene* with `room` (e.g. "gui3D_room") at the origin, identity rotation (Scene vtable slot SceneVt_Init).
// Returns SF_None, SF_SceneBad (Scene* unreadable or null), SF_VtableBad (its vtable slot unreadable) or SF_Unknown (null view /
// room, or an offset unknown for this exe). *sceneOut = the Scene* read (0 if none), for logging.
inline int initScene(void* view, const char* room, uintptr_t* sceneOut = nullptr) {
    using namespace detail;
    if (sceneOut) *sceneOut = 0;
    const int oObj = off(&K2AddrTable::View_SceneObj), oInit = off(&K2AddrTable::SceneVt_Init);
    if (!view || !room || !oObj || !oInit || !g_sv.callSceneInit) return SF_Unknown;
    uint32_t sc = 0;
    bool ok = rd<uint32_t>(u(view), oObj, &sc);
    if (sceneOut) *sceneOut = sc;
    if (!ok || !sc || !saferead::probeRead(SiteBase, reinterpret_cast<const void*>(static_cast<uintptr_t>(sc)), 4)) return SF_SceneBad;
    uint32_t vt = 0, fn = 0;
    if (!rd<uint32_t>(sc, 0, &vt) || !rd<uint32_t>(vt, oInit, &fn)) return SF_VtableBad;
    float pos[3] = {0, 0, 0}, quat[4] = {0, 0, 0, 1};
    g_sv.callSceneInit(fn, sc, room, pos, quat);
    return SF_None;
}

// Number of models in the view's scene, or -1 when unreadable / unknown.
inline int modelCount(void* view) {
    int n = -1, o = detail::off(&K2AddrTable::View_ModelCount);
    if (!view || !o || !detail::rd<int>(detail::u(view), o, &n)) return -1;
    return n;
}
// Scene model `idx` (0 = the first model added, e.g. the light rig), or nullptr.
inline void* getModel(void* view, int idx) {
    void* sc = detail::scene(view);
    return sc && g_sv.getModel ? g_sv.getModel(sc, idx) : nullptr;
}
// Adds the model `resref` at `idx` (-1 = append). Returns the engine's result (the model, or nullptr).
inline void* addModel(void* view, const char* resref, int idx = -1) {
    void* sc = detail::scene(view);
    if (!sc || !resref || !g_sv.addModel || !g_sv.exoCtor || !g_sv.exoDtor) return nullptr;
    char exo[16] = {0};
    g_sv.exoCtor(exo, resref);
    void* m = g_sv.addModel(sc, exo, idx);
    g_sv.exoDtor(exo);
    return m;
}
// Removes model `idx` (deleteIt = also destroy it). False when the view / call is unknown.
inline bool removeModel(void* view, int idx, bool deleteIt = true) {
    void* sc = detail::scene(view);
    if (!sc || !g_sv.removeModel) return false;
    g_sv.removeModel(sc, idx, deleteIt ? 1 : 0);
    return true;
}
// The view's camera object and its vtable (each 0 when unreadable). True only when the vtable is the expected camera vtable.
inline bool camera(void* view, uint32_t* cam, uint32_t* camVt) {
    using namespace detail;
    uint32_t c = 0, v = 0; const int o = off(&K2AddrTable::View_Camera); const uint32_t want = vtab(&K2AddrTable::SceneCameraVtable);
    if (view && o) rd<uint32_t>(u(view), o, &c);
    if (c) rd<uint32_t>(c, 0, &v);
    if (cam) *cam = c;
    if (camVt) *camVt = v;
    return c && want && v == want;
}

// Attaches the camera to `node` of `model` (camera vtable slot SceneCamVt_Attach). False (nothing called) when the camera is not the
// expected camera object, the offset is unknown or the slot is unreadable / zero.
inline bool cameraAttach(void* view, void* model, const char* node, int flag) {
    uint32_t cam = 0, v = 0, fn = 0;
    const int o = detail::off(&K2AddrTable::SceneCamVt_Attach);
    if (!camera(view, &cam, &v) || !o || !detail::rd<uint32_t>(v, o, &fn) || !fn || !g_sv.callCamAttach) return false;
    g_sv.callCamAttach(fn, cam, model, node, flag);
    return true;
}
// Sets the camera field of view (camera vtable slot SceneCamVt_Fov); same refusal rules as cameraAttach.
inline bool cameraFov(void* view, float fov) {
    uint32_t cam = 0, v = 0, fn = 0;
    const int o = detail::off(&K2AddrTable::SceneCamVt_Fov);
    if (!camera(view, &cam, &v) || !o || !detail::rd<uint32_t>(v, o, &fn) || !fn || !g_sv.callCamFov) return false;
    g_sv.callCamFov(fn, cam, fov);
    return true;
}

// ---- scene models (Gob) ----
// Vtable guard before any Gob vtable call: the Gob (GobReadable bytes) and its vtable (GobVtReadable bytes) must be readable and
// every checked slot must hold the expected function. Returns GF_None when safe. checks (optional, GobSlotCount entries) receives
// every slot's offset / expected / actual value / name; *nBad = mismatching slots; *vtOut = the vtable read (0 if none).
inline int gobCheck(void* gob, uint32_t* vtOut = nullptr, GobSlotCheck* checks = nullptr, int* nBad = nullptr) {
    using namespace detail;
    if (vtOut) *vtOut = 0;
    if (nBad) *nBad = 0;
    const int gobLen = off(&K2AddrTable::GobReadable), vtLen = off(&K2AddrTable::GobVtReadable);
    const GobSlotCheck want[GobSlotCount] = {
        {off(&K2AddrTable::GobVt_SetPosition), vtab(&K2AddrTable::GobSetPosition), 0, "SetPosition"},
        {off(&K2AddrTable::GobVt_SetOrientation), vtab(&K2AddrTable::GobSetOrientation), 0, "SetOrientation"},
        {off(&K2AddrTable::GobVt_GetPosition), vtab(&K2AddrTable::GobGetPosition), 0, "GetPosition"},
        {off(&K2AddrTable::GobVt_GetOrientation), vtab(&K2AddrTable::GobGetOrientation), 0, "GetOrientation"},
        {off(&K2AddrTable::GobVt_GetParent), vtab(&K2AddrTable::GobGetParent), 0, "GetParent"}};
    if (gobLen <= 0 || vtLen <= 0) return GF_Unknown;
    for (const auto& w : want) if (!w.off || !w.expect) return GF_Unknown;
    if (!gob || !saferead::probeRead(SiteBase, gob, gobLen)) return GF_Unreadable;
    uint32_t vt = 0;
    rd<uint32_t>(u(gob), 0, &vt);
    if (vtOut) *vtOut = vt;
    if (!vt || !saferead::probeRead(SiteBase, reinterpret_cast<const void*>(static_cast<uintptr_t>(vt)), vtLen)) return GF_VtUnreadable;
    int bad = 0;
    for (int i = 0; i < GobSlotCount; ++i) {
        GobSlotCheck c = want[i];
        rd<uint32_t>(vt, c.off, &c.got);
        if (c.got != c.expect) ++bad;
        if (checks) checks[i] = c;
    }
    if (nBad) *nBad = bad;
    return bad ? GF_Slot : GF_None;
}

// Pose readback: pos = (0,0,0) and q = (w=1,0,0,0) unless the getters run. Call only after gobCheck passed.
inline bool gobGetPose(void* gob, float* pos, float* q) {
    using namespace detail;
    pos[0] = pos[1] = pos[2] = q[1] = q[2] = q[3] = 0; q[0] = 1;
    uint32_t fp = gobSlot(gob, off(&K2AddrTable::GobVt_GetPosition)), fq = gobSlot(gob, off(&K2AddrTable::GobVt_GetOrientation));
    if (!fp || !fq || !g_sv.callGetVec) return false;
    g_sv.callGetVec(fp, gob, pos);
    g_sv.callGetVec(fq, gob, q);
    return true;
}
inline bool gobSetPosition(void* gob, float x, float y, float z) {
    uint32_t fn = detail::gobSlot(gob, detail::off(&K2AddrTable::GobVt_SetPosition));
    if (!fn || !g_sv.callSetPos) return false;
    g_sv.callSetPos(fn, gob, x, y, z);
    return true;
}
inline bool gobSetOrientation(void* gob, float w, float x, float y, float z) {
    uint32_t fn = detail::gobSlot(gob, detail::off(&K2AddrTable::GobVt_SetOrientation));
    if (!fn || !g_sv.callSetOri) return false;
    g_sv.callSetOri(fn, gob, w, x, y, z);
    return true;
}
// Parent Gob (nullptr = none, the normal case for a scene model). False when the slot is unknown / unreadable.
inline bool gobGetParent(void* gob, void** parent, int arg = -1) {
    *parent = nullptr;
    uint32_t fn = detail::gobSlot(gob, detail::off(&K2AddrTable::GobVt_GetParent));
    if (!fn || !g_sv.callGetParent) return false;
    *parent = g_sv.callGetParent(fn, gob, arg);
    return true;
}
// Uniform scale at Gob +Gob_Scale.
inline bool gobGetScale(void* gob, float* s) {
    int o = detail::off(&K2AddrTable::Gob_Scale);
    return gob && o && detail::rd<float>(detail::u(gob), o, s);
}
inline bool gobSetScale(void* gob, float s) {
    int o = detail::off(&K2AddrTable::Gob_Scale);
    if (!gob || !o) return false;
    void* p = static_cast<unsigned char*>(gob) + o;
    if (!saferead::probeWrite(detail::SiteBase, p, sizeof(float))) return false;
    *static_cast<float*>(p) = s;
    return true;
}

} // namespace sceneview
