/* HolsterApi.h -- the "k2.holster" v1 interface (provider: the holster patch) + a C++ consumer helper.
 *
 * Why: the holster-editor patch (in-game Options screen) hosts the editor (doc 42 HW-EDIT): its 3D preview (k2.preview3d) shows
 * weapons holstered and the placement is edited live. Holster owns the tune data, the mount pose and holster.ini; holster-editor
 * owns the UI and requires this interface.
 *
 * Calls run on the game's main thread (the editor panel tick); holster applies registered preview Gobs from its per-frame
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

/* Edit scopes (BeginEdit / SetScope). Body scope edits an additive per-body delta: offset and, since holster 0.4.0-dev, rotation (place_rot_body_<name>). */
#define K2HOLSTER_SCOPE_WEAPON 0
#define K2HOLSTER_SCOPE_TYPE   1
#define K2HOLSTER_SCOPE_BODY   2

/* K2HolsterEditInfo.src[]: where the shown value comes from. */
#define K2HOLSTER_SRC_GLOBAL 0
#define K2HOLSTER_SRC_TYPE   1
#define K2HOLSTER_SRC_WEAPON 2
#define K2HOLSTER_SRC_BODY   4   /* bit in src[1]: a per-body offset delta is added */
#define K2HOLSTER_SRC_BODYROT 16 /* bit in src[0] (holster 0.4.0-dev): a non-zero per-body rotation delta is composed into rot; older editors ignore it */

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
    void    (KPATCH_CALL *NudgeWorld)(const float dq[4], const float dp[3]);           /* world-space delta, dq w-first (body scope: the rotation turns the per-body delta) */
    void    (KPATCH_CALL *NudgeLocal)(int32_t axis, float deg, float metres);          /* bone axis 0 x right, 1 y forward, 2 z up */
    void    (KPATCH_CALL *SetValues)(const float rot[3], const float off[3]);          /* absolute values (body scope: the shown rot and off; the delta to the base is the body delta) */
    int32_t (KPATCH_CALL *CycleBone)(int32_t dir);                                     /* returns the new bone index (1-based) */
    int32_t (KPATCH_CALL *GetEdit)(K2HolsterEditInfo* out);                            /* 1 while an edit is open */
    int32_t (KPATCH_CALL *GetMountWorld)(float pos[3], float q[4]);                    /* world pose of the edited Gob's mount bone */
    int32_t (KPATCH_CALL *GetNodeWorld)(void* gob, const char* node, float pos[3], float q[4]); /* e.g. the preview rig's camerahook */
    int32_t (KPATCH_CALL *SetScope)(int32_t scope, int32_t left);                      /* implicit Revert; the caller confirms Discard first */
    int32_t (KPATCH_CALL *Save)(void);                                                 /* 1 written to holster.ini */
    void    (KPATCH_CALL *Revert)(void);                                               /* overlay back to the saved values */
    void    (KPATCH_CALL *ResetScope)(void);                                           /* delete this scope's keys, then save */
    void    (KPATCH_CALL *EndEdit)(void);                                              /* drop the overlay */
    int32_t (KPATCH_CALL *SetPreviewDraw)(int32_t drawn);                              /* holster 0.8.0-dev: 1 = the preview weapons slide to the hand (draw), 0 = back to the mount (stow); returns the state now shown. Field newer than K2HOLSTER_V1_BASE_SIZE: check struct_size */
} K2HolsterApi;

/* struct_size of the first 16-pointer table (everything before SetPreviewDraw): the minimum a consumer accepts. */
#define K2HOLSTER_V1_BASE_SIZE (16 * sizeof(void*))

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
K2HOLSTER_STATIC_ASSERT(offsetof(K2HolsterApi, SetPreviewDraw) == K2HOLSTER_V1_BASE_SIZE, "K2HolsterApi.SetPreviewDraw offset");
K2HOLSTER_STATIC_ASSERT(sizeof(K2HolsterApi) == 17 * sizeof(void*), "K2HolsterApi size");
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
        return p && p->struct_size >= K2HOLSTER_V1_BASE_SIZE && p->RegisterPreview && p->UnregisterPreview && p->BeginEdit && p->NudgeWorld &&
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

    // True when the provider is new enough for the preview draw / stow toggle (holster 0.8.0-dev+).
    bool canDraw() const { return connected && h && h->struct_size >= sizeof(K2HolsterApi) && h->SetPreviewDraw; }

    void poll() { if (!connected && (pollCount++ & 255) == 0) connect(); }
    bool ok() const { return connected && h; }
};

} // namespace k2holster
#endif

#endif /* K2_HOLSTERAPI_H */
