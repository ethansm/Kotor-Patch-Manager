// common_contract_test.cpp -- Patches/Common CONTRACT test for npc-world-probe.
// Pins what NpcWorldProbe.cpp relies on in SafeRead.h (beginScope, readAt of the three creature fields it logs: +0x11AC, +0x11FC, +0x90,
// failure on null / unreadable creatures), KpLog.h (fail-soft init, the 232-byte line cut against the longest probe line) and NpcWorld.h
// (a client without a provider stays inert, so a missing npc-world cannot crash the probe). Every message starts with
// "npc-world-probe relies on: ". Host-only, deterministic, no game. Offsets are independent literals (doc 40 section 7.5), never read back.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "SafeRead.h"
#include "KpLog.h"
#include "NpcWorld.h"
#include "ProbeCore.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("npc-world-probe relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("npc-world-probe relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("npc-world-probe");
KP_CHANNEL(chPersist, "persist");

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    uint8_t* arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", arena != nullptr && arena != MAP_FAILED);
    g_stubVirtualQueryAll = true;   // host stub: all memory reads as one committed read-write region
    const uintptr_t A = reinterpret_cast<uintptr_t>(arena);

    // 1. creature fields logged for persistence evidence: area id @+0x90, @+0x11AC, @+0x11FC (SaveCreatures skips a creature when either is nonzero)
    {
        const uint32_t c = 0x200;
        const uint32_t areaId = 0x7F000005u, f1 = 0, f2 = 0xDEADBEEFu;
        memcpy(arena + c + 0x90, &areaId, 4); memcpy(arena + c + 0x11ac, &f1, 4); memcpy(arena + c + 0x11fc, &f2, 4);
        saferead::beginScope();
        unsigned a = 1, b = 1, id = 0;
        HCHECK("readAt<unsigned> at creature+0x11AC", saferead::readAt(0x7401, A + c, 0x11ac, &a));
        HCHECK("readAt<unsigned> at creature+0x11FC", saferead::readAt(0x7401, A + c, 0x11fc, &b));
        HCHECK("readAt<unsigned> at creature+0x90", saferead::readAt(0x7401, A + c, 0x90, &id));
        HEQ("+0x11AC value", (long long)a, (long long)f1); HEQ("+0x11FC value", (long long)b, (long long)f2); HEQ("+0x90 value", (long long)id, (long long)areaId);
        HCHECK("a null creature fails readAt (the event pointer is only valid during the callback)", !saferead::readAt(0x7401, 0, 0x11ac, &a));
        g_stubVirtualQueryAll = false; saferead::beginScope();
        HCHECK("an unreadable creature fails readAt (probe logs 'fields unreadable')", !saferead::readAt(0x7401, A + c, 0x11ac, &a));
        g_stubVirtualQueryAll = true; saferead::beginScope();
        HCHECK("beginScope drops the cached verdict", saferead::readAt(0x7401, A + c, 0x11ac, &a));
    }

    // 2. KpLog: the longest line the probe writes must stay under the core's 232-byte cut, through the real KP_LOG path
    {
        fakekp::reset(); fakekp::withLog();
        HEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
        nwp::Env* none = nullptr; (void)none;
        char line[nwp::MaxLine];
        snprintf(line, sizeof line, "RESTORED oid=%08x tag=\"%s\" area=%s pos=%.1f,%.1f,%.1f f11ac=%08x f11fc=%08x", 0xFFFFFFFFu, std::string(32, 't').c_str(), std::string(16, 'a').c_str(),
                 -99999.9f, -99999.9f, -99999.9f, 0xFFFFFFFFu, 0xFFFFFFFFu);
        KP_LOGV(chPersist, KPLOG_INFO, "%s", line);
        HCHECK("KP_LOGV reaches the core service", !fakekp::state().logLines.empty());
        const std::string got = fakekp::state().logLines.empty() ? std::string() : fakekp::state().logLines.back();
        const size_t c2 = got.find(':', got.find(':') + 1);   // fakekp lines are "<channel>:<level>:<text>"
        const std::string text = c2 == std::string::npos ? std::string() : got.substr(c2 + 1);
        HCHECK("the longest probe line arrives uncut", text.find("f11fc=ffffffff") != std::string::npos);
        HCHECK("the Probe line buffer (MaxLine) is below the 232-byte cut", nwp::MaxLine < 232);
    }

    // 3. NpcWorld.h: with no provider the probe's client is inert and reports not connected
    {
        fakekp::reset();
        k2nw::Client c; c.init(fakekp::api(), "npc-world-probe");
        HCHECK("connect() without npc-world is false", !c.connect());
        HEQ("spawn() without npc-world = 0", (long long)c.spawn("c_jawa", "t", 0, 0, 0, 0), 0LL);
        HCHECK("tagPresent() without npc-world is false", !c.tagPresent("t"));
        HEQ("spawnCap() without npc-world = 0", c.spawnCap(), 0);
    }
    return kt::Report("common_contract_test");
}
