/* HolsterApi.h -- the "k2.holster" v1 interface (provider: the holster patch) + a C++ consumer helper.
 *
 * Why: the equip-screen 3D preview (equipment-3d-character) shows weapons holstered and hosts the holster placement editor
 * (doc 42 HW-EDIT). Holster owns the tune data, the mount pose and holster.ini; equip3d owns the UI. Equip3d requires this
 * interface OPTIONALLY: when it is absent, the holster toggle and the gear never appear and the preview works as before.
 *
 * Calls run on the game's main thread (the equip panel tick); holster applies registered preview Gobs from its per-frame
 * attach-refresh hook (0x504EF5). Every Gob passed to RegisterPreview must be unregistered before it is freed.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints, floats and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 */
#ifndef K2_HOLSTERAPI_H
#define K2_HOLSTERAPI_H

#include "kpatch_api.h"
#include <stddef.h>
#include <stdint.h>

#define K2HOLSTER_IFACE   "k2.holster"
#define K2HOLSTER_VERSION 1

/* Edit scopes (BeginEdit / SetScope). Body scope edits the offset only (an additive per-body delta). */
#define K2HOLSTER_SCOPE_WEAPON 0
#define K2HOLSTER_SCOPE_TYPE   1
#define K2HOLSTER_SCOPE_BODY   2

/* K2HolsterEditInfo.src[]: where the shown value comes from. */
#define K2HOLSTER_SRC_GLOBAL 0
#define K2HOLSTER_SRC_TYPE   1
#define K2HOLSTER_SRC_WEAPON 2
#define K2HOLSTER_SRC_BODY   4   /* bit: a per-body delta is added (offset only) */

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2HolsterEditInfo {
    uint32_t struct_size;
    char model[17], type[12], body[17], bone[12];   /* bone: "custom" when the boot bone_name is not one of the five editor bones */
    int32_t scope, left, dirty;
    int32_t hidden;                                  /* 1: a more specific level overrides this scope (the edit is not what shows in world) */
    float rot[3], off[3];                            /* rot: degrees, continuous with the previous GetEdit (no euler flips); off: metres, bone frame */
    uint8_t src[3];                                  /* rot, off, bone: K2HOLSTER_SRC_* */
} K2HolsterEditInfo;

typedef struct K2HolsterApi {
    uint32_t struct_size;
    int32_t (KPATCH_CALL *RegisterPreview)(void* gob, void* beh_or_0, int32_t left);   /* 1 ok, 0 registry full / invalid */
    void    (KPATCH_CALL *UnregisterPreview)(void* gob);                               /* gob 0 = clear all; ends an edit on that gob */
    int32_t (KPATCH_CALL *BeginEdit)(void* gob, int32_t scope);                        /* 1 ok; gob must be registered */
    void    (KPATCH_CALL *NudgeWorld)(const float dq[4], const float dp[3]);           /* world-space delta, dq w-first (rotation ignored in body scope) */
    void    (KPATCH_CALL *NudgeLocal)(int32_t axis, float deg, float metres);          /* bone axis 0 x right, 1 y forward, 2 z up */
    void    (KPATCH_CALL *SetValues)(const float rot[3], const float off[3]);          /* absolute values (body scope: absolute offset) */
    int32_t (KPATCH_CALL *CycleBone)(int32_t dir);                                     /* returns the new bone index (1-based) */
    int32_t (KPATCH_CALL *GetEdit)(K2HolsterEditInfo* out);                            /* 1 while an edit is open */
    int32_t (KPATCH_CALL *GetMountWorld)(float pos[3], float q[4]);                    /* world pose of the edited Gob's mount bone */
    int32_t (KPATCH_CALL *GetNodeWorld)(void* gob, const char* node, float pos[3], float q[4]); /* e.g. the preview rig's camerahook */
    int32_t (KPATCH_CALL *SetScope)(int32_t scope, int32_t left);                      /* implicit Revert; the caller confirms Discard first */
    int32_t (KPATCH_CALL *Save)(void);                                                 /* 1 written to holster.ini */
    void    (KPATCH_CALL *Revert)(void);                                               /* overlay back to the saved values */
    void    (KPATCH_CALL *ResetScope)(void);                                           /* delete this scope's keys, then save */
    void    (KPATCH_CALL *EndEdit)(void);                                              /* drop the overlay */
} K2HolsterApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(__cplusplus)
#define K2HOLSTER_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define K2HOLSTER_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef K2HOLSTER_STATIC_ASSERT
K2HOLSTER_STATIC_ASSERT(offsetof(K2HolsterApi, RegisterPreview) == sizeof(void*), "K2HolsterApi.RegisterPreview offset");
K2HOLSTER_STATIC_ASSERT(offsetof(K2HolsterApi, EndEdit) == 15 * sizeof(void*), "K2HolsterApi.EndEdit offset");
K2HOLSTER_STATIC_ASSERT(sizeof(K2HolsterApi) == 16 * sizeof(void*), "K2HolsterApi size");
#undef K2HOLSTER_STATIC_ASSERT
#endif

#ifdef __cplusplus
namespace k2holster {

// Optional consumer: connect() lazily, poll() every frame (retries every 256 calls until connected). ok() false = feature hidden.
struct Client {
    const KPatchApi* api = nullptr;
    const K2HolsterApi* h = nullptr;
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;
    bool reportedMissing = false;

    void init(const KPatchApi* a) {
        api = a;
        h = nullptr;
        connected = false;
        pollCount = 0;
        reportedMissing = false;
    }

    static bool usable(const K2HolsterApi* p) {
        return p && p->struct_size >= sizeof(K2HolsterApi) && p->RegisterPreview && p->UnregisterPreview && p->BeginEdit && p->NudgeWorld &&
               p->NudgeLocal && p->SetValues && p->CycleBone && p->GetEdit && p->GetMountWorld && p->GetNodeWorld && p->SetScope && p->Save &&
               p->Revert && p->ResetScope && p->EndEdit;
    }

    bool connect() {
        if (connected && h) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2HolsterApi* p = static_cast<const K2HolsterApi*>(api->require(K2HOLSTER_IFACE, K2HOLSTER_VERSION));
        if (!usable(p)) {
            if (!reportedMissing && onConnect) onConnect(false, p ? "k2.holster v1 table too small" : "k2.holster v1 not provided");
            reportedMissing = true;
            return false;
        }
        h = p;
        connected = true;
        if (onConnect) onConnect(true, "connected");
        return true;
    }

    void poll() { if (!connected && (pollCount++ & 255) == 0) connect(); }
    bool ok() const { return connected && h; }
};

} // namespace k2holster
#endif

#endif /* K2_HOLSTERAPI_H */
