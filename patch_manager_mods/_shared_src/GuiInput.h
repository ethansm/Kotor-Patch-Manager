/* GuiInput.h -- the "k2.guiinput" v1 registry interface (provider: the gui-input patch) + a C++ consumer helper.
 *
 * Why: 4 separate mods (Action Hotbar, Inventory 3D Viewport, Equipment 3D Character Preview, Character Active Effects Panel)
 * each subclass the game window (SetWindowLongPtrA GWLP_WNDPROC) to intercept WM_MOUSEWHEEL. Unsubclassing is fragile ("only if still
 * on top" guard), leaving dangling pointers if unloaded out of sequence.
 * gui-input consolidates the 4 stacked subclasses into a single provider that owns the window hook, tracks foreground state,
 * transforms cursor coordinates to engine virtual pixels, and dispatches wheel events to registered subscribers.
 * Design: patch_manager_mods/18_mouse_camera_investigation.md:358, 24_master_plan_status.md:68.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 */
#ifndef K2_GUIINPUT_H
#define K2_GUIINPUT_H

#include "kpatch_api.h"
#include <stddef.h>
#include <stdint.h>

#define K2GUIINPUT_IFACE   "k2.guiinput"
#define K2GUIINPUT_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2WheelEvent {
    uint32_t struct_size;
    int32_t  delta;       /* mouse wheel delta: +120 away/forward, -120 toward user/back */
    float    engine_x;    /* cursor x in engine virtual pixels (top-left origin) */
    float    engine_y;    /* cursor y in engine virtual pixels */
    int32_t  client_x;    /* cursor x in client window pixels */
    int32_t  client_y;    /* cursor y in client window pixels */
    int32_t  screen_w;    /* engine screen width */
    int32_t  screen_h;    /* engine screen height */
} K2WheelEvent;

/* Callback returns 1 (non-zero) if consumed (swallows the event), 0 to pass through to other subscribers / game */
typedef int32_t (KPATCH_CALL *K2WheelCallback)(const K2WheelEvent* ev, void* user);

typedef struct K2GuiInputApi {
    uint32_t struct_size;
    /* HWND of the game window, or 0 if not yet found */
    intptr_t (KPATCH_CALL *GameHwnd)(void);
    /* Converts current cursor pos to engine virtual coordinates (top-left). Returns 1 if valid, 0 if unavailable. */
    int32_t  (KPATCH_CALL *CursorEngine)(float* x, float* y);
    /* Subscribe to mouse wheel events. Returns token > 0 on success, <= 0 on failure (null cb, table full). */
    int32_t  (KPATCH_CALL *WheelSubscribe)(K2WheelCallback cb, void* user);
    /* Unsubscribe all registrations matching cb. */
    void     (KPATCH_CALL *WheelUnsubscribe)(K2WheelCallback cb);
    /* Returns 1 if the game window is the foreground window, 0 otherwise. */
    int32_t  (KPATCH_CALL *Foreground)(void);
} K2GuiInputApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

// ---- C++ consumer helper ------------------------------------------------------------------------------------------------------
// In KPatchInit:   g_input.init(api, "my-patch-id"); g_input.connect();
// In your code:    g_input.poll();
//                  float x, y; if (g_input.cursor_engine(&x, &y)) ...
//                  g_input.wheel_subscribe(onWheel, myData);
// Without a provider (not installed or config order), calls are safe fallbacks/no-ops.
#ifdef __cplusplus
namespace k2input {

struct Client {
    const KPatchApi* api = nullptr;
    const K2GuiInputApi* gi = nullptr;
    const char* owner = "";
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;

    void init(const KPatchApi* pApi, const char* ownerId) {
        api = pApi;
        owner = ownerId ? ownerId : "";
        gi = nullptr;
        connected = false;
        pollCount = 0;
    }

    bool connect() {
        if (connected && gi) return true;
        if (!api || !api->require) {
            if (onConnect) onConnect(false, "no KPatchApi / require");
            return false;
        }
        if (api->struct_size < sizeof(KPatchApi)) {
            if (onConnect) onConnect(false, "short KPatchApi struct_size");
            return false;
        }
        const void* p = api->require(K2GUIINPUT_IFACE, K2GUIINPUT_VERSION);
        if (!p) {
            if (onConnect) onConnect(false, "provider not found");
            return false;
        }
        const auto* iface = static_cast<const K2GuiInputApi*>(p);
        if (iface->struct_size < sizeof(K2GuiInputApi)) {
            if (onConnect) onConnect(false, "short K2GuiInputApi struct_size");
            return false;
        }
        gi = iface;
        connected = true;
        if (onConnect) onConnect(true, "connected");
        return true;
    }

    void poll() {
        if (connected) return;
        if ((++pollCount & 0xFF) == 0) connect();
    }

    intptr_t game_hwnd() const { return (gi && gi->GameHwnd) ? gi->GameHwnd() : 0; }

    bool cursor_engine(float* x, float* y) const {
        return (gi && gi->CursorEngine) ? (gi->CursorEngine(x, y) != 0) : false;
    }

    int32_t wheel_subscribe(K2WheelCallback cb, void* user) {
        return (gi && gi->WheelSubscribe) ? gi->WheelSubscribe(cb, user) : 0;
    }

    void wheel_unsubscribe(K2WheelCallback cb) {
        if (gi && gi->WheelUnsubscribe) gi->WheelUnsubscribe(cb);
    }

    bool foreground() const {
        return (gi && gi->Foreground) ? (gi->Foreground() != 0) : false;
    }
};

} // namespace k2input
#endif

#endif // K2_GUIINPUT_H
