// common_contract_test.cpp -- Patches/Common CONTRACT test for voice-probe (0.2.0 hotkey trigger).
// Pins what VoiceProbe.cpp relies on in Party.h (controlledCreature -> serverCreature chain, failure reasons, no engine call on a broken
// chain), SafeRead.h (readAt of the server creature's object id at +0x04, failure on null / unreadable), NpcWorld.h (a client without
// a provider stays inert and accepts a K2NW_TICK registration) and KpLog.h (the longest trigger line arrives uncut). Every message starts
// with "voice-probe relies on: ". Host-only, deterministic, no game. Offsets are independent literals, never read back from the mod.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "SafeRead.h"
#include "Party.h"
#include "KpLog.h"
#include "NpcWorld.h"
#include "TriggerCore.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("voice-probe relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("voice-probe relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("voice-probe");
KP_CHANNEL(chMain, "main");

static uint8_t* g_arena = nullptr;
static size_t g_used = 0;
static uintptr_t arenaAlloc(size_t n) { uintptr_t p = reinterpret_cast<uintptr_t>(g_arena) + g_used; g_used += (n + 15) & ~size_t(15); memset(reinterpret_cast<void*>(p), 0, n); return p; }
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

static uintptr_t g_client = 0, g_server = 0;
static int g_gcCalls = 0, g_scCalls = 0;
static uintptr_t __thiscall fakeGetControlled(void*) { ++g_gcCalls; return g_client; }
static uintptr_t __thiscall fakeServer(void* m) { ++g_scCalls; return reinterpret_cast<uintptr_t>(m) == g_client ? g_server : 0; }

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    void* m = mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0);
    HCHECK("arena allocated", m != MAP_FAILED);
    if (m == MAP_FAILED) return kt::Report("common_contract_test (voice-probe)");
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;

    // 1. Party.h: app -> +4 -> +4 -> +0x270 = party table (count > 0) -> controlled client creature -> server creature
    const uintptr_t cell = arenaAlloc(16), app = arenaAlloc(16), pa = arenaAlloc(16), pb = arenaAlloc(0x300), tbl = arenaAlloc(16);
    g_server = arenaAlloc(16);
    g_client = arenaAlloc(16);
    at<uint32_t>(cell, 0) = (uint32_t)app; at<uint32_t>(app, 4) = (uint32_t)pa; at<uint32_t>(pa, 4) = (uint32_t)pb;
    at<uint32_t>(pb, 0x270) = (uint32_t)tbl; at<int>(tbl, 0) = 1;
    at<uint32_t>(g_server, 4) = 0x00000123u;   // server object id (CSWSObject +0x04)
    party::g_appGlobal = cell; party::g_getControlled = fakeGetControlled; party::g_serverCreature = fakeServer;
    {
        saferead::beginScope();
        party::Fail f = party::F_App;
        const uintptr_t c = party::controlledCreature(&f);
        HEQ("controlledCreature returns the engine's controlled client creature", (unsigned long long)c, (unsigned long long)g_client);
        HEQ("controlledCreature reports F_None", (int)f, (int)party::F_None);
        HEQ("serverCreature maps the client member to its server creature", (unsigned long long)party::serverCreature(c), (unsigned long long)g_server);
        HEQ("serverCreature(0) = 0 without an engine call", (unsigned long long)(g_scCalls = 0, party::serverCreature(0)), 0ull);
        HEQ("serverCreature(0) makes no call", g_scCalls, 0);
        uint32_t id = 0;
        HCHECK("readAt<uint32_t> server+0x04 (object id)", saferead::readAt<uint32_t>(0x6C40, g_server, 4, &id) && id == 0x123u);
        HCHECK("readAt on a null server creature fails", !saferead::readAt<uint32_t>(0x6C40, 0, 4, &id));
        g_stubVirtualQueryAll = false; saferead::beginScope();
        HCHECK("readAt on unreadable memory fails (trigger logs 'server creature id unreadable')", !saferead::readAt<uint32_t>(0x6C40, g_server, 4, &id));
        g_stubVirtualQueryAll = true; saferead::beginScope();
    }
    {
        at<int>(tbl, 0) = 0; g_gcCalls = 0;
        party::Fail f = party::F_None;
        HEQ("party count 0 -> controlledCreature 0", (unsigned long long)party::controlledCreature(&f), 0ull);
        HEQ("party count 0 -> F_Count", (int)f, (int)party::F_Count);
        HEQ("party count 0 -> no engine call", g_gcCalls, 0);
        HCHECK("failText is a non-empty static string (trigger skip=<reason>)", party::failText(f) && party::failText(f)[0]);
        at<int>(tbl, 0) = 1;
        at<uint32_t>(cell, 0) = 0;
        HEQ("null app global -> 0", (unsigned long long)party::controlledCreature(&f), 0ull);
        HEQ("null app global -> F_App", (int)f, (int)party::F_App);
        at<uint32_t>(cell, 0) = (uint32_t)app;
    }

    // 2. NpcWorld.h: without npc-world the client accepts the tick registration and stays inert
    {
        fakekp::reset();
        k2nw::Client c; c.init(fakekp::api(), "voice-probe");
        HCHECK("add(K2NW_TICK) accepted", c.add(K2NW_TICK, [](const K2NwEvent*, void*) {}));
        HCHECK("K2NW_TICK is inside K2NW_ALL", (K2NW_TICK & K2NW_ALL) != 0);
        HCHECK("connect() without npc-world is false", !c.connect());
        HCHECK("not connected", !c.connected);
    }

    // 3. KpLog: the longest trigger line arrives uncut through KP_LOG
    {
        fakekp::reset(); fakekp::withLog();
        HEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
        char line[vpt::MaxLine];
        snprintf(line, sizeof line, "TRIGGER #%ld t=%lu script=%s oid=%08lx called=%d ret=%d", 2147483647L, 4294967295ul, "abcdefghijklmnop", 0xFFFFFFFFul, 1, -2147483647 - 1);
        KP_LOG(chMain, "%s", line);
        HCHECK("KP_LOG reaches the core service", !fakekp::state().logLines.empty());
        const std::string got = fakekp::state().logLines.empty() ? std::string() : fakekp::state().logLines.back();
        HCHECK("the longest trigger line arrives uncut", got.find("ret=-2147483648") != std::string::npos);
        HCHECK("MaxLine is below the 232-byte core cut", vpt::MaxLine < 232);
    }
    return kt::Report("common_contract_test (voice-probe)");
}
