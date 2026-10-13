// common_contract_test.cpp -- Patches/Common CONTRACT test for npc-world.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins the semantics NpcWorldProvider.cpp relies on in SafeRead.h (beginScope, readAt on creature fields, the tag pointer chain, the
// 16-byte area resref probeRead, failure on null / tiny / unreadable pointers) and KpLog.h (KpLog_Init fail-soft, KpLog_HasCore, the
// 232-byte line cut vs the longest event line). Every message starts with "npc-world relies on: ". Host-only, deterministic, no game.
// Expected offsets are independent literals (doc 38 sections 13/14, area-hook-probe 0.2.0 logs), never read back from the code under test.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "SafeRead.h"
#include "GameAddr.h"
#include "KpLog.h"
#include "NpcWorld.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("npc-world relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("npc-world relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("npc-world");
KP_CHANNEL(chAdd, "add");

static uint8_t* g_arena;

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    g_arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", g_arena != nullptr && g_arena != MAP_FAILED);
    g_stubVirtualQueryAll = true;   // host stub: all memory reads as one committed read-write region
    const uintptr_t A = reinterpret_cast<uintptr_t>(g_arena);

    // 0. GameAddr.h: the engine operator new the native spawn allocates the creature with (doc 38 section 13)
    HEQ("gameaddr::New = 0x919723", (long long)gameaddr::New, 0x919723LL);

    // 1. creature fields read by NW_OnAddToArea: id @+4, position floats @+0x94/+0x98/+0x9c, tag chain [c+0x24] -> +0x18 -> char*
    {
        const uint32_t c = 0x100, tagObj = 0x400, tagStr = 0x500;
        const uint32_t id = 0x7F000123u; memcpy(g_arena + c + 4, &id, 4);
        const float px = 1.5f, py = -2.25f, pz = 0.0f;
        memcpy(g_arena + c + 0x94, &px, 4); memcpy(g_arena + c + 0x98, &py, 4); memcpy(g_arena + c + 0x9c, &pz, 4);
        uint32_t tp = uint32_t(A + tagObj); memcpy(g_arena + c + 0x24, &tp, 4);
        uint32_t sp = uint32_t(A + tagStr); memcpy(g_arena + tagObj + 0x18, &sp, 4);
        memcpy(g_arena + tagStr, "k1p_native_npc", 15);
        saferead::beginScope();
        unsigned rid = 0; float rx = 0, ry = 0, rz = 9;
        HCHECK("readAt<unsigned> at creature+4 reads the object id", saferead::readAt(0x7301, A + c, 4, &rid));
        HEQ("object id value", (long long)rid, (long long)id);
        HCHECK("readAt<float> reads +0x94/+0x98/+0x9c", saferead::readAt(0x7306, A + c, 0x94, &rx) && saferead::readAt(0x7306, A + c, 0x98, &ry) && saferead::readAt(0x7306, A + c, 0x9c, &rz));
        HCHECK("position values", rx == 1.5f && ry == -2.25f && rz == 0.0f);
        unsigned t1 = 0, t2 = 0;
        HCHECK("readAt<unsigned> at creature+0x24 yields the tag object", saferead::readAt(0x7303, A + c, 0x24, &t1) && t1 == tp);
        HCHECK("readAt<unsigned> at tagObj+0x18 yields the char*", saferead::readAt(0x7303, A + tagObj, 0x18, &t2) && t2 == sp);
        HCHECK("probeRead(1) on the char* succeeds and the bytes are the tag", saferead::probeRead(0x7304, g_arena + tagStr, 1) && strcmp(reinterpret_cast<const char*>(g_arena + tagStr), "k1p_native_npc") == 0);
        HCHECK("a null creature fails readAt (hook argument can be garbage)", !saferead::readAt(0x7301, 0, 4, &rid));
    }

    // 2. area resref: 16 bytes at area+0x110, not necessarily NUL-terminated
    {
        const uint32_t area = 0x1000;
        memset(g_arena + area + 0x110, 0, 16); memcpy(g_arena + area + 0x110, "204tel", 6);
        saferead::beginScope();
        HCHECK("probeRead(16) at area+0x110 succeeds", saferead::probeRead(0x7305, g_arena + area + 0x110, 16));
        HCHECK("resref bytes", memcmp(g_arena + area + 0x110, "204tel\0", 7) == 0);
        HCHECK("probeRead below 0x10000 fails (a garbage small area pointer)", !saferead::probeRead(0x7305, reinterpret_cast<const void*>(0x1234 + 0x110), 16));
        HCHECK("probeRead on null fails", !saferead::probeRead(0x7305, nullptr, 16));
        g_stubVirtualQueryAll = false; saferead::beginScope();
        HCHECK("an unreadable resref fails probeRead (provider reports area \"\")", !saferead::probeRead(0x7305, g_arena + area + 0x110, 16));
        g_stubVirtualQueryAll = true; saferead::beginScope();
        HCHECK("beginScope drops the cached verdict so memory readable again reads again", saferead::probeRead(0x7305, g_arena + area + 0x110, 16));
    }

    // 3. KpLog: fail-soft init, HasCore, and the longest event line must stay under the core's 232-byte cut
    {
        HEQ("KpLog_Init(nullptr) = 0", KpLog_Init(nullptr), 0);
        fakekp::reset();
        HEQ("KpLog_Init without kpatch.log service = 0", KpLog_Init(fakekp::api()), 0);
        HCHECK("KpLog_HasCore is false without the service", !KpLog_HasCore());
        KP_LOG(chAdd, "ADD no core yet");   // fallback sink, must not crash
        fakekp::withLog();
        HEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
        HCHECK("KpLog_HasCore is true with the service", KpLog_HasCore());
        // the longest line the provider writes: ADD with a 32-char tag, 16-char area and large coordinates
        KP_LOG(chAdd, "ADD c=%08lx id=%08x tag=\"%s\" area=%s pos=%.1f,%.1f,%.1f origin=%s", 0xFFFFFFFFul, 0xFFFFFFFFu,
               std::string(32, 't').c_str(), std::string(16, 'a').c_str(), -99999.9f, -99999.9f, -99999.9f, "engine");
        HCHECK("KP_LOG reaches the core service", !fakekp::state().logLines.empty());
        const std::string line = fakekp::state().logLines.empty() ? std::string() : fakekp::state().logLines.back();
        HCHECK("the longest ADD line arrives uncut", line.find("origin=engine") != std::string::npos);
        const size_t c2 = line.find(':', line.find(':') + 1);   // fakekp lines are "<channel>:<level>:<text>"
        const std::string text = c2 == std::string::npos ? std::string() : line.substr(c2 + 1);
        HCHECK("the longest ADD line is shorter than the 232-byte cut", !text.empty() && text.size() < 232);
    }

    // 4. NpcWorld.h: a consumer with no provider stays inert (npc-world missing / config order wrong)
    {
        fakekp::reset();
        k2nw::Client c; c.init(fakekp::api(), "t");
        HCHECK("connect() without provider is false", !c.connect());
        char b[8]; b[0] = 'x';
        HEQ("currentArea() without provider = 0", c.currentArea(b, sizeof b), 0);
        HEQ("... and writes an empty string", int(b[0]), 0);
        HEQ("visitSalt() without provider = 0", (long long)c.visitSalt(), 0LL);
        HEQ("spawn() without provider = 0", (long long)c.spawn("r", "t", 0, 0, 0, 0), 0LL);
    }
    return kt::Report("common_contract_test");
}
