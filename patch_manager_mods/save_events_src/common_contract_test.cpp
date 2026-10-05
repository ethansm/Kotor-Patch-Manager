// common_contract_test.cpp -- Patches/Common CONTRACT test for save-events.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins the semantics SaveEventsProvider.cpp relies on in SafeRead.h (beginScope, the app -> request -> CExoString read chain, readExo
// length / printable rules, probeRead on a char*), GameAddr.h (AppGlobal) and KpLog.h (KpLog_Init fail-soft, the 232-byte line cut vs the
// longest event line). Every message starts with "save-events relies on: ". Host-only, deterministic, no game, no DLL.
// Expected values are independent literals (the hooks toml, the hotbar's DB lessons 355-357), never read back from the header under test.
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
#include "SaveEvents.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("save-events relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("save-events relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("save-events");
KP_CHANNEL(chEvt, "evt");

static uint8_t* g_arena;
static void putExo(uint32_t at, uint32_t strAt, const char* s, int len) {   // CExoString {char* p @+0; int len @+4}, len includes the NUL
    memcpy(g_arena + strAt, s, strlen(s) + 1);
    uint32_t p = uint32_t(reinterpret_cast<uintptr_t>(g_arena + strAt));
    memcpy(g_arena + at, &p, 4); memcpy(g_arena + at + 4, &len, 4);
}

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    g_arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", g_arena != nullptr && g_arena != MAP_FAILED);
    g_stubVirtualQueryAll = true;   // host stub: all memory reads as one committed read-write region
    const uintptr_t A = reinterpret_cast<uintptr_t>(g_arena);

    // 1. GameAddr.h: the global the save request chain starts from (hooks toml / hotbar DB lessons 355-357)
    HEQ("gameaddr::AppGlobal = 0xA1B4A4", (long long)gameaddr::AppGlobal, 0xA1B4A4LL);

    // 2. SafeRead: app = *(AppGlobal); R = *(app+0x14); folder CExoString at R+0x18 (readAt<int> chain, then readExo)
    {
        const uint32_t appObj = 0x100, reqObj = 0x200, exoAt = reqObj + 0x18;
        uint32_t reqPtr = uint32_t(A + reqObj);
        memcpy(g_arena + appObj + 0x14, &reqPtr, 4);
        putExo(exoAt, 0x400, "SAVES:000012 - Game11", 22);       // engine lengths include the trailing NUL
        saferead::beginScope();
        int req = 0; char raw[128]; raw[0] = 0;
        HCHECK("readAt<int> at app+0x14 yields the request pointer", saferead::readAt(0x7201, A + appObj, 0x14, &req));
        HEQ("request pointer value", (long long)(unsigned)req, (long long)reqPtr);
        HEQ("readExo at R+0x18 returns the string length without the NUL", saferead::readExo(0x7202, uintptr_t(req) + 0x18, raw, sizeof raw, 120), 21);
        HCHECK("readExo content", strcmp(raw, "SAVES:000012 - Game11") == 0);
        HCHECK("readAt with a null base fails (app global still 0 before the game starts)", !saferead::readAt(0x7201, 0, 0x14, &req));

        // length rules the provider's calls (maxLen 120 / cap 128, maxLen 150 / cap 160) depend on
        std::string longName(100, 'a'); putExo(exoAt, 0x500, longName.c_str(), 101);
        HEQ("a 100-char name is accepted with maxLen 120", saferead::readExo(0x7202, A + exoAt - 0, raw, sizeof raw, 120), 100);
        std::string tooLong(121, 'b'); putExo(exoAt, 0x600, tooLong.c_str(), 122);
        HEQ("len 122 (121 chars + NUL) is rejected with maxLen 120", saferead::readExo(0x7202, A + exoAt, raw, sizeof raw, 120), 0);
        HEQ("a rejected read leaves an empty buffer", int(raw[0]), 0);
        putExo(exoAt, 0x700, "SAVES:000012 - G", 0);             // len 0 = empty CExoString
        HEQ("len 0 is rejected", saferead::readExo(0x7202, A + exoAt, raw, sizeof raw, 120), 0);
        putExo(exoAt, 0x700, "SAVES:000012 - G", 17);
        HEQ("len >= cap is rejected (the buffer must hold the NUL)", saferead::readExo(0x7202, A + exoAt, raw, 17, 120), 0);
        HEQ("len < cap is accepted", saferead::readExo(0x7202, A + exoAt, raw, 18, 120), 16);
        g_arena[0x700 + 4] = 0x01;                               // control byte
        HEQ("a non-printable byte is rejected (printableOnly default)", saferead::readExo(0x7202, A + exoAt, raw, sizeof raw, 120), 0);
        std::string root = "cloudsaves/76561198000000001/";
        putExo(0x800, 0x900, root.c_str(), int(root.size()) + 1);
        HEQ("the cloud root CExoString (maxLen 150) reads whole", saferead::readExo(0x7202, A + 0x800, raw, sizeof raw, 150), (long long)root.size());
        HCHECK("cloud root keeps its trailing slash", raw[root.size() - 1] == '/');
    }

    // 3. SafeRead: the byte-wise char* walk used for the LoadGame parameter, and the cloud flag byte
    {
        saferead::beginScope();
        memcpy(g_arena + 0xA00, "SAVES:000003 - Q", 17);
        HCHECK("probeRead(1) succeeds on mapped memory", saferead::probeRead(0x7204, g_arena + 0xA00, 1));
        HCHECK("probeRead on a null pointer fails", !saferead::probeRead(0x7204, nullptr, 1));
        HCHECK("probeRead below 0x10000 fails (a garbage small pointer)", !saferead::probeRead(0x7204, reinterpret_cast<const void*>(0x1234), 1));
        uint8_t flag = 0xFF; g_arena[0xB00] = 1;
        HCHECK("readAt<uint8_t> reads the cloud flag", saferead::readAt(0x7203, A + 0xB00, 0, &flag));
        HEQ("cloud flag value", int(flag), 1);
        flag = 9; g_stubVirtualQueryAll = false; saferead::beginScope();
        HCHECK("an unreadable flag fails", !saferead::readAt(0x7203, A + 0xB00, 0, &flag));
        HEQ("a failed readAt leaves the flag untouched (provider treats it as 'not cloud' only via its own 0 init)", int(flag), 9);
        HCHECK("an unreadable char* fails probeRead", !saferead::probeRead(0x7204, g_arena + 0xA00, 1));
        g_stubVirtualQueryAll = true; saferead::beginScope();
    }

    // 4. KpLog: fail-soft init, and the longest event line must stay under the core's 232-byte cut
    {
        HEQ("KpLog_Init(nullptr) = 0", KpLog_Init(nullptr), 0);
        fakekp::reset();
        HEQ("KpLog_Init without kpatch.log service = 0", KpLog_Init(fakekp::api()), 0);
        KP_LOG(chEvt, "NEW_GAME consumers=%d", 0);   // no core yet: fallback sink, must not crash
        fakekp::withLog();
        HEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
        KP_LOG(chEvt, "%s folder='%.150s' cloud=%d consumers=%d", "LOAD_REQUEST", std::string(150, 'f').c_str(), 1, 16);
        HCHECK("KP_LOG reaches the core service", !fakekp::state().logLines.empty());
        const std::string line = fakekp::state().logLines.empty() ? std::string() : fakekp::state().logLines.back();
        HCHECK("the longest event line (150-char folder) arrives uncut", line.find("consumers=16") != std::string::npos);
        const size_t c2 = line.find(':', line.find(':') + 1);   // fakekp lines are "<channel>:<level>:<text>"
        const std::string text = c2 == std::string::npos ? std::string() : line.substr(c2 + 1);
        HCHECK("the longest event line is shorter than the 232-byte cut", !text.empty() && text.size() < 232);
    }

    // 5. SaveEvents.h: a consumer with no provider stays inert (save-events missing / config order wrong)
    {
        fakekp::reset();
        k2se::Client c; c.init(fakekp::api(), "t");
        HCHECK("connect() without provider is false", !c.connect());
        char b[8]; b[0] = 'x';
        HEQ("currentSaveFolder() without provider = 0", c.currentSaveFolder(b, sizeof b), 0);
        HEQ("... and writes an empty string", int(b[0]), 0);
    }

    munmap(g_arena, 0x10000);
    return ::kt::Report("common_contract_test (save-events)");
}
