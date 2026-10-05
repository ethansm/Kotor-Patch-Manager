// common_contract_test.cpp -- Patches/Common CONTRACT test for hud-host.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins semantics relied on by HudHostProvider.cpp in SafeRead.h (beginScope, readAt<int8_t> at menu+0x1BAA) and KpLog.h
// (KpLog_Init with no / short api, KP_LOG without a core), plus HudHost.h's k2hud::Client being inert without a provider.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SafeRead.h"
#include "KpLog.h"
#include "HudHost.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("hud-host relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("hud-host relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("hud-host");
KP_CHANNEL(chCore, "core");

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    uint8_t* arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", arena != nullptr && arena != MAP_FAILED);
    uintptr_t menu = reinterpret_cast<uintptr_t>(arena);

    // 1. SafeRead: int8 target type at menu+0x1BAA (0..3 normal, 0xFF = none), and a failed read leaves the output alone
    g_stubVirtualQueryAll = true;   // host stub: all memory reads as one committed read-write region
    saferead::beginScope();
    uint8_t raw = 2;
    memcpy(arena + 0x1BAA, &raw, 1);
    int8_t tt = -1;
    HCHECK("readAt<int8_t> succeeds on mapped memory", saferead::readAt(0x7001, menu, 0x1BAA, &tt));
    HEQ("readAt<int8_t> value", int(tt), 2);
    raw = 0xFF; memcpy(arena + 0x1BAA, &raw, 1);
    saferead::beginScope();   // a fresh scope per hook call, as the provider does
    tt = 0;
    HCHECK("readAt<int8_t> reads 0xFF", saferead::readAt(0x7001, menu, 0x1BAA, &tt));
    HEQ("0xFF reads back as -1 (sign extended int8)", int(tt), -1);
    int8_t untouched = 5;
    HCHECK("readAt at null base fails", !saferead::readAt(0x7001, 0, 0x1BAA, &untouched));
    HEQ("failed readAt leaves out untouched", int(untouched), 5);
    g_stubVirtualQueryAll = false; saferead::beginScope();
    HCHECK("readAt fails when the region is not committed/readable", !saferead::readAt(0x7001, menu, 0x1BAA, &untouched));
    HEQ("unreadable readAt leaves out untouched", int(untouched), 5);

    // 2. KpLog: KpLog_Init refuses a null api and an api without require; logging with no core must not crash
    HEQ("KpLog_Init(nullptr) = 0", KpLog_Init(nullptr), 0);
    fakekp::reset();
    HEQ("KpLog_Init without kpatch.log service = 0", KpLog_Init(fakekp::api()), 0);
    KP_LOG(chCore, "no core yet %d", 1);   // fallback sink (INFO is below the default WARN level: dropped)
    fakekp::withLog();
    HEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
    KP_LOG(chCore, "provided %s v%d", K2HUD_IFACE, K2HUD_VERSION);
    HCHECK("KP_LOG reaches the core service", !fakekp::state().logLines.empty());

    // 3. HudHost.h: a consumer with no provider stays inert (hud-host missing / config order wrong)
    fakekp::reset();
    k2hud::Client c; c.init(fakekp::api(), "t");
    HCHECK("connect() without provider is false", !c.connect());
    HEQ("hud() without provider = 0", c.hud(), 0);
    HEQ("frame() without provider = 0", int(c.frame()), 0);

    munmap(arena, 0x10000);
    return ::kt::Report("common_contract_test (hud-host)");
}
