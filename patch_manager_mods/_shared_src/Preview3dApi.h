/* Preview3dApi.h -- the "k2.preview3d" v1 interface (provider: the equipment-3d-character preview patch) + a C++ consumer helper.
 *
 * Why: the equip-screen 3D character preview is the one place that builds a creature for a UI panel: the Gob, the weapon and
 * helmet attach order, the vtable-slot guards and the view maths. Other mods that need a live model in a panel (the holster
 * placement editor, doc 42 HW-EDIT, and later panels) host it through this interface instead of copying the engine calls.
 * The provider owns the view block, the creature, the weapon and helmet Gobs and the camera maths. A consumer names the creature
 * to show (Bind), builds it (Rebuild) and drives yaw, zoom, pan, weapons and animation.
 *
 * Tokens: CreateView returns a uint32_t token; 0 means failure. Every other call takes the token. A token stays valid until the
 * panel that owns its view is destroyed. An unknown or stale token makes the call a no-op (0 / void) and never crashes.
 * A consumer may keep a token across frames. It must not keep a Gob pointer across a Rebuild or Destroy (see Gob lifetime below).
 *
 * Threading: calls run on the game's main thread (the panel ctor and the panel tick), as in K2HolsterApi.
 *
 * CreateView scope: valid only while the panel's layout is open, i.e. inside the panel ctor (SceneView.h:118-119). NOT enforced
 * in code: a call outside that window is a consumer bug.
 *
 * Destroy order: the BeforeFree callback runs first, while every Gob is still attached and alive. Then the weapon and helmet Gobs
 * are detached and freed, then the creature. Destroy keeps the view state (yaw, zoom, panZ) and the cached appearance and
 * alignment. It clears only the built creature, so Bind survives and a later Rebuild builds from the same source creature.
 *
 * Rebuild order: the weapon and helmet Gobs are detached before any step that can fail. A Rebuild that returns 0 has therefore
 * already removed them.
 *
 * Gob lifetime: a Gob from WeaponGob or RigGob is valid until the next Rebuild, SetWeapons(off), or Destroy on that token.
 * A consumer that registers a Gob elsewhere (holster RegisterPreview) must unregister it in its BeforeFree callback.
 *
 * Size guard (one rule, both sides): before reading any field of KPatchApi, check
 *   api->struct_size >= offsetof(KPatchApi, require) + sizeof(api->require)
 * This is the same check the provider makes in KPatchInit (GuiInputProvider.cpp:218). Before using any K2Preview3dApi function, the
 * consumer checks struct_size >= the end of ViewControl (the 12 original slots) and that every one of them is non-null (Client::usable).
 * Slots appended after ViewControl (SetAnimPaused, provider 0.2.0) are OPTIONAL: an editor built against this header still connects to
 * an older provider (shorter table), and an older editor works against a newer provider (longer table). Check Client::canPause()
 * before calling SetAnimPaused.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints, floats and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 */
#ifndef K2_PREVIEW3DAPI_H
#define K2_PREVIEW3DAPI_H

#include "kpatch_api.h"
#include <stddef.h>
#include <stdint.h>

#define K2PREVIEW3D_IFACE   "k2.preview3d"
#define K2PREVIEW3D_VERSION 1

/* WeaponGob hand. */
#define K2PREVIEW3D_HAND_RIGHT 0
#define K2PREVIEW3D_HAND_LEFT  1

/* RigGob part. */
#define K2PREVIEW3D_PART_BODY 0   /* body Gob: the camerahook node lives here */
#define K2PREVIEW3D_PART_HEAD 1

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

/* Fired before any of the token's weapon Gobs is freed (Destroy, view teardown, Rebuild, SetWeapons), while WeaponGob still returns
 * them, so the consumer can unregister them (k2.holster UnregisterPreview). May fire more than once per operation. Up to 4
 * (cb, user) registrations; every one fires. */
typedef void (KPATCH_CALL *K2Preview3dBeforeFree)(uint32_t token, void* user);

typedef struct K2Preview3dApi {
    uint32_t struct_size;
    /* Adds a 3D scene view bound to layout control `tag` of `panel` (the panel object pointer). Valid only inside the panel ctor.
     * Returns a token, or 0 when panel/tag is null, an engine address is unknown, or the allocation failed. */
    uint32_t (KPATCH_CALL *CreateView)(void* panel, const char* tag);
    /* Sets the real creature to show (the one SetStats would use). Builds nothing: call Rebuild. 1 ok, 0 unknown token or null creature. */
    int32_t  (KPATCH_CALL *Bind)(uint32_t token, void* creature);
    /* Builds the preview from the bound creature: appearance copy, scene, idle, view, weapons when on, helmet. 1 built, 0 failed
     * (see the Rebuild order above). */
    int32_t  (KPATCH_CALL *Rebuild)(uint32_t token);
    /* Weapons shown (stance) on or off. Attaches or detaches the weapon Gobs; the view state is untouched. */
    void     (KPATCH_CALL *SetWeapons)(uint32_t token, int32_t on);
    /* K2PREVIEW3D_HAND_*: the attached weapon Gob, or 0 when none. Holster registers these as preview Gobs. */
    void*    (KPATCH_CALL *WeaponGob)(uint32_t token, int32_t hand);
    /* Camera and facing: yaw in degrees, zoom (divides the fov through fovForZoom, clamped to 150 deg), panZ in metres (creature
     * Z offset). The creature position is re-sent only when panZ changed. */
    void     (KPATCH_CALL *SetView)(uint32_t token, float yaw, float zoom, float panZ);
    /* Plays `anim` on the body and head parts (cross-fade in). Null or empty = no-op. "idle" = the rig's idle name; Rebuild does not play it. */
    void     (KPATCH_CALL *PlayAnim)(uint32_t token, const char* anim);
    /* K2PREVIEW3D_PART_*: the body or head rig Gob, or 0 when nothing is built. */
    void*    (KPATCH_CALL *RigGob)(uint32_t token, int32_t part);
    /* Frees the creature and its attached Gobs (BeforeFree first, then weapons and helmet, then the creature). Keeps yaw, zoom,
     * panZ and the cached appearance and alignment; the token and its Bind stay valid. */
    void     (KPATCH_CALL *Destroy)(uint32_t token);
    /* Adds a BeforeFree (cb, user) registration (a repeat is a no-op; up to 4). cb 0 removes every registration of `user`. */
    void     (KPATCH_CALL *SetBeforeFree)(K2Preview3dBeforeFree cb, void* user);
    /* The CSWGui3DSceneView control CreateView bound (the GUI control the consumer lists with appendToPanel when the layout control
     * had ID -1, and resizes with its rect slot), or 0 for an unknown token. */
    void*    (KPATCH_CALL *ViewControl)(uint32_t token);
    /* ---- appended after v1 shipped (additive, K2PREVIEW3D_VERSION stays 1; slots 12+ are optional: see Client::canPause) ---- */
    /* paused != 0 freezes the body and head animation at its CURRENT pose (no restart at frame 0, no slow motion); 0 resumes from that
     * same pose. Idempotent (pausing a paused view or resuming a running one is a no-op). The weapon Gobs ride the frozen body, so the
     * pose can be inspected for clipping. A PlayAnim always starts the new clip running and clears the paused state (it never starts
     * paused: re-send SetAnimPaused(token, 1) after it if the pause should survive a clip change). Rebuild and Destroy also clear it.
     * No-op for an unknown token, an unbuilt view, or before the first PlayAnim (nothing to freeze). The helmet visor animation is not
     * frozen. */
    void     (KPATCH_CALL *SetAnimPaused)(uint32_t token, int32_t paused);
} K2Preview3dApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(__cplusplus)
#define K2PREVIEW3D_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define K2PREVIEW3D_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef K2PREVIEW3D_STATIC_ASSERT
K2PREVIEW3D_STATIC_ASSERT(offsetof(K2Preview3dApi, CreateView) == sizeof(void*), "K2Preview3dApi.CreateView offset");
K2PREVIEW3D_STATIC_ASSERT(offsetof(K2Preview3dApi, SetBeforeFree) == 10 * sizeof(void*), "K2Preview3dApi.SetBeforeFree offset");
K2PREVIEW3D_STATIC_ASSERT(offsetof(K2Preview3dApi, ViewControl) == 11 * sizeof(void*), "K2Preview3dApi.ViewControl offset");
K2PREVIEW3D_STATIC_ASSERT(offsetof(K2Preview3dApi, SetAnimPaused) == 12 * sizeof(void*), "K2Preview3dApi.SetAnimPaused offset");
K2PREVIEW3D_STATIC_ASSERT(sizeof(K2Preview3dApi) == 13 * sizeof(void*), "K2Preview3dApi size");
#undef K2PREVIEW3D_STATIC_ASSERT
#endif

#ifdef __cplusplus
namespace k2p3 {

// Optional consumer. init(api) once; poll() every frame. connect() is tried on the first poll and then on every 256th call
// until it succeeds. ok() false = the preview is unavailable (the panel should hide it).
struct Client {
    const KPatchApi* api = nullptr;
    const K2Preview3dApi* p = nullptr;
    bool connected = false;
    unsigned pollCount = 0;
    bool reportedMissing = false;
    void (*onConnect)(bool ok, const char* why) = nullptr;

    void init(const KPatchApi* a) {
        api = a;
        p = nullptr;
        connected = false;
        pollCount = 0;
        reportedMissing = false;
    }

    // Size of the original 12-slot v1 table (through ViewControl). A provider older than SetAnimPaused still reports at least this.
    static constexpr size_t kBaseSize = offsetof(K2Preview3dApi, ViewControl) + sizeof(void*);
    // Size a table needs for SetAnimPaused to be readable.
    static constexpr size_t kPauseSize = offsetof(K2Preview3dApi, SetAnimPaused) + sizeof(void*);

    // Every ORIGINAL v1 field the consumer calls is present and non-null, and the table covers at least those 12 slots. Newer
    // appended slots are optional (canPause).
    static bool usable(const K2Preview3dApi* t) {
        return t && t->struct_size >= kBaseSize && t->CreateView && t->Bind && t->Rebuild && t->SetWeapons && t->WeaponGob &&
               t->SetView && t->PlayAnim && t->RigGob && t->Destroy && t->SetBeforeFree && t->ViewControl;
    }

    bool connect() {
        if (connected && p) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2Preview3dApi* t = static_cast<const K2Preview3dApi*>(api->require(K2PREVIEW3D_IFACE, K2PREVIEW3D_VERSION));
        if (!usable(t)) {
            if (!reportedMissing && onConnect) onConnect(false, t ? "k2.preview3d v1 table too small" : "k2.preview3d v1 not provided");
            reportedMissing = true;
            return false;
        }
        p = t;
        connected = true;
        if (onConnect) onConnect(true, "connected");
        return true;
    }

    void poll() {
        if (!connected && (pollCount++ & 255) == 0) connect();
    }
    bool ok() const { return connected && p; }
    // The connected provider also has SetAnimPaused (provider 0.2.0+). Never read the slot without this.
    bool canPause() const { return connected && p && p->struct_size >= kPauseSize && p->SetAnimPaused; }
};

} // namespace k2p3
#endif

#endif /* K2_PREVIEW3DAPI_H */
