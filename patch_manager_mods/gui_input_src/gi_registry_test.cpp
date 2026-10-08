// gi_registry_test.cpp -- Host test of k2input::Client and K2GuiInputApi ABI layout.
// Verifies inert-without-provider, connect(), poll(), struct_size gating, and ABI layout.
#include "GuiInput.h"
#include "check.h"
#include "FakeKPatchApi.h"

// ABI layout checks
#if UINTPTR_MAX == 0xffffffff
static_assert(sizeof(K2GuiInputApi) == 24, "K2GuiInputApi size 32-bit");
static_assert(offsetof(K2GuiInputApi, struct_size) == 0, "api.struct_size");
static_assert(offsetof(K2GuiInputApi, GameHwnd) == 4, "api.GameHwnd");
static_assert(offsetof(K2GuiInputApi, CursorEngine) == 8, "api.CursorEngine");
static_assert(offsetof(K2GuiInputApi, WheelSubscribe) == 12, "api.WheelSubscribe");
static_assert(offsetof(K2GuiInputApi, WheelUnsubscribe) == 16, "api.WheelUnsubscribe");
static_assert(offsetof(K2GuiInputApi, Foreground) == 20, "api.Foreground");
#endif

static_assert(sizeof(K2WheelEvent) == 32, "K2WheelEvent size");
static_assert(offsetof(K2WheelEvent, struct_size) == 0, "ev.struct_size");
static_assert(offsetof(K2WheelEvent, delta) == 4, "ev.delta");
static_assert(offsetof(K2WheelEvent, engine_x) == 8, "ev.engine_x");
static_assert(offsetof(K2WheelEvent, engine_y) == 12, "ev.engine_y");
static_assert(offsetof(K2WheelEvent, client_x) == 16, "ev.client_x");
static_assert(offsetof(K2WheelEvent, client_y) == 20, "ev.client_y");
static_assert(offsetof(K2WheelEvent, screen_w) == 24, "ev.screen_w");
static_assert(offsetof(K2WheelEvent, screen_h) == 28, "ev.screen_h");

#define GCHECK(why, expr) ::kt::Check(why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define GEQ(why, a, b) ::kt::CheckEq(why " (line " KT_STR(__LINE__) ")", (a), (b))

// Fake provider implementation
static int g_gameHwndCalls = 0;
static int g_cursorCalls = 0;
static int g_subCalls = 0;
static int g_unsubCalls = 0;
static int g_fgCalls = 0;

static intptr_t KPATCH_CALL fakeGameHwnd() { ++g_gameHwndCalls; return 0x12345678; }
static int32_t KPATCH_CALL fakeCursorEngine(float* x, float* y) {
    ++g_cursorCalls;
    if (x) *x = 400.0f;
    if (y) *y = 300.0f;
    return 1;
}
static int32_t KPATCH_CALL fakeWheelSub(K2WheelCallback, void*) { ++g_subCalls; return 42; }
static void KPATCH_CALL fakeWheelUnsub(K2WheelCallback) { ++g_unsubCalls; }
static int32_t KPATCH_CALL fakeForeground() { ++g_fgCalls; return 1; }

static const K2GuiInputApi g_fakeTable = {
    sizeof(K2GuiInputApi),
    fakeGameHwnd,
    fakeCursorEngine,
    fakeWheelSub,
    fakeWheelUnsub,
    fakeForeground
};

static int32_t KPATCH_CALL dummyCb(const K2WheelEvent*, void*) { return 0; }

int main() {
    fakekp::reset();

    // 1. Inert without provider
    k2input::Client client;
    client.init(fakekp::api(), "test_mod");
    GEQ("not connected initially", client.connected, false);
    GEQ("connect without provider returns false", client.connect(), false);
    GEQ("game_hwnd without provider is 0", client.game_hwnd(), 0);
    float cx = 0, cy = 0;
    GEQ("cursor_engine without provider returns false", client.cursor_engine(&cx, &cy), false);
    GEQ("wheel_subscribe without provider returns 0", client.wheel_subscribe(dummyCb, nullptr), 0);
    client.wheel_unsubscribe(dummyCb); // safe no-op
    GEQ("foreground without provider returns false", client.foreground(), false);

    // 2. Short KPatchApi struct_size gating
    fakekp::reset(8); // too small for KPatchApi
    client.init(fakekp::api(), "test_mod");
    GEQ("connect with short KPatchApi fails", client.connect(), false);

    // 3. Short K2GuiInputApi struct_size gating
    fakekp::reset();
    K2GuiInputApi shortTable = g_fakeTable;
    shortTable.struct_size = 8;
    fakekp::provide(K2GUIINPUT_IFACE, K2GUIINPUT_VERSION, &shortTable);
    client.init(fakekp::api(), "test_mod");
    GEQ("connect with short provider struct_size fails", client.connect(), false);

    // 4. Successful connect
    fakekp::reset();
    fakekp::provide(K2GUIINPUT_IFACE, K2GUIINPUT_VERSION, &g_fakeTable);
    client.init(fakekp::api(), "test_mod");
    GCHECK("connect with valid provider succeeds", client.connect());
    GEQ("connected is true", client.connected, true);

    // 5. Provider calls routed correctly
    GEQ("game_hwnd routes to provider", client.game_hwnd(), 0x12345678);
    GEQ("gameHwndCalls count", g_gameHwndCalls, 1);

    GCHECK("cursor_engine routes to provider", client.cursor_engine(&cx, &cy));
    GEQ("cursorCalls count", g_cursorCalls, 1);
    GCHECK("cx returned is 400.0f", cx == 400.0f);
    GCHECK("cy returned is 300.0f", cy == 300.0f);

    GEQ("wheel_subscribe routes to provider", client.wheel_subscribe(dummyCb, nullptr), 42);
    GEQ("subCalls count", g_subCalls, 1);

    client.wheel_unsubscribe(dummyCb);
    GEQ("unsubCalls count", g_unsubCalls, 1);

    GEQ("foreground routes to provider", client.foreground(), true);
    GEQ("fgCalls count", g_fgCalls, 1);

    // 6. Polling cadence test
    fakekp::reset();
    client.init(fakekp::api(), "test_mod");
    GEQ("not connected", client.connected, false);
    // 255 polls should not connect yet
    for (int i = 1; i <= 255; ++i) client.poll();
    GEQ("still disconnected after 255 polls", client.connected, false);

    // Now inject provider and poll one more time (total 256 polls)
    fakekp::provide(K2GUIINPUT_IFACE, K2GUIINPUT_VERSION, &g_fakeTable);
    client.poll();
    GEQ("connected at poll 256", client.connected, true);

    return ::kt::Report("gi_registry_test");
}
