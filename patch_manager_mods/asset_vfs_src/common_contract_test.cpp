// common_contract_test.cpp -- Patches/Common CONTRACT test for asset-vfs.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins the semantics AssetVfsProvider.cpp relies on in GameAddr.h, SafeRead.h, KpLog.h, NpcWorld.h,
// VfsRegistry.h, TwoDaSplicer.h, and CharacterProvider.h.
// Every message starts with "asset-vfs relies on: ". Host-only, deterministic, no game.
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
#include "VfsRegistry.h"
#include "TwoDaSplicer.h"
#include "CharacterProvider.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("asset-vfs relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("asset-vfs relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("asset-vfs");
KP_CHANNEL(chCore, "core");

static uint8_t* g_arena;

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    g_arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", g_arena != nullptr && g_arena != MAP_FAILED);
    g_stubVirtualQueryAll = true;
    const uintptr_t A = reinterpret_cast<uintptr_t>(g_arena);

    // -------------------------------------------------------------
    // 1. GameAddr.h: exact engine addresses used by asset-vfs
    // -------------------------------------------------------------
    HEQ("gameaddr::ResManGlobal = 0x00A1B490", (long long)gameaddr::ResManGlobal, 0x00A1B490LL);
    HEQ("gameaddr::ResManMalloc = 0x00712F30", (long long)gameaddr::ResManMalloc, 0x00712F30LL);
    HEQ("gameaddr::ResManFreeData = 0x007120C0", (long long)gameaddr::ResManFreeData, 0x007120C0LL);
    HEQ("gameaddr::ExoCtor = 0x00733570", (long long)gameaddr::ExoCtor, 0x00733570LL);
    HEQ("gameaddr::ExoStringAssign = 0x007337C0", (long long)gameaddr::ExoStringAssign, 0x007337C0LL);
    HEQ("gameaddr::ExoDtor = 0x00733780", (long long)gameaddr::ExoDtor, 0x00733780LL);
    HEQ("gameaddr::New = 0x00919723", (long long)gameaddr::New, 0x00919723LL);

    // -------------------------------------------------------------
    // 2. SafeRead.h: beginScope, readAt, probeRead, and null checks
    // -------------------------------------------------------------
    {
        const uint32_t off = 0x200;
        uint32_t val = 0xDEADBEEF;
        memcpy(g_arena + off, &val, 4);

        saferead::beginScope();
        uint32_t readVal = 0;
        HCHECK("readAt reads valid memory", saferead::readAt(0x7410, A, off, &readVal));
        HEQ("readAt value matches", (long long)readVal, (long long)val);

        HCHECK("probeRead succeeds on readable range", saferead::probeRead(0x7414, g_arena + off, 16));
        HCHECK("readAt fails on null pointer", !saferead::readAt(0x7410, 0, 0, &readVal));
        HCHECK("probeRead fails on null pointer", !saferead::probeRead(0x7414, nullptr, 16));
    }

    // -------------------------------------------------------------
    // 3. KpLog.h: fail-soft init, HasCore, and channel logging
    // -------------------------------------------------------------
    {
        HEQ("KpLog_Init(nullptr) = 0", KpLog_Init(nullptr), 0);
        fakekp::reset();
        HEQ("KpLog_Init without service = 0", KpLog_Init(fakekp::api()), 0);
        HCHECK("KpLog_HasCore is false without service", !KpLog_HasCore());

        fakekp::withLog();
        HEQ("KpLog_Init with service = 1", KpLog_Init(fakekp::api()), 1);
        HCHECK("KpLog_HasCore is true with service", KpLog_HasCore());

        KP_LOG(chCore, "test log message");
        HCHECK("KP_LOG reaches log buffer", !fakekp::state().logLines.empty());
    }

    // -------------------------------------------------------------
    // 4. NpcWorld.h: client helper and inert defaults
    // -------------------------------------------------------------
    {
        fakekp::reset();
        k2nw::Client client;
        client.init(fakekp::api(), "asset-vfs");
        HCHECK("connect without provider is false", !client.connect());
        HEQ("spawn without provider returns 0", (long long)client.spawn("test_res", "test_tag", 0, 0, 0, 0), 0LL);
        HEQ("tagPresent without provider is false", client.tagPresent("tag") ? 1 : 0, 0);
    }

    // -------------------------------------------------------------
    // 5. VfsRegistry.h: packed ID prefix, struct sizes, registry, and cursors
    // -------------------------------------------------------------
    {
        HEQ("kVfsPackedPrefix = 0x40FC0000u", (long long)vfsreg::kVfsPackedPrefix, 0x40FC0000LL);
        HEQ("sizeof(KeyTableEntry) = 0x1C", (long long)sizeof(vfsreg::KeyTableEntry), 0x1CLL);
        HEQ("sizeof(FakeKeyTable) = 0x20", (long long)sizeof(vfsreg::FakeKeyTable), 0x20LL);

        vfsreg::Registry reg;
        const uint8_t dummyData[] = "dummy";
        int slot = reg.registerAsset("test_asset", vfsreg::ResTypeUtc, dummyData, sizeof(dummyData));
        HCHECK("registerAsset succeeds", slot >= 0);
        HCHECK("findResMan finds registered asset", reg.findResMan("test_asset", vfsreg::ResTypeUtc) != nullptr);

        vfsreg::MilesCursorTable cursors;
        uint32_t h = cursors.open(dummyData, sizeof(dummyData));
        HCHECK("MilesCursorTable opens handle", h != 0);
        HCHECK("isVfsHandle confirms VFS handle", vfsreg::MilesCursorTable::isVfsHandle(h));
        HCHECK("close handle succeeds", cursors.close(h));
    }

    // -------------------------------------------------------------
    // 6. TwoDaSplicer.h: ParsedView and spliceRows
    // -------------------------------------------------------------
    {
        const char* fake2da = "2DA V2.b\ncol1\tcol2\n2\n0\t1\t\0\0\0\0\x01\x00\0"; // minimal header
        twoda::ParsedView pv;
        // Verify ParsedView parse rejects truncated buffer gracefully
        HCHECK("parse truncated buffer returns false", !pv.parse(reinterpret_cast<const uint8_t*>(fake2da), 7));
    }

    // -------------------------------------------------------------
    // 7. CharacterProvider.h: interfaces, ABI layout, and Catalog
    // -------------------------------------------------------------
    {
        HCHECK("K2_ASSETVFS_IFACE string", strcmp(K2_ASSETVFS_IFACE, "k2.assetvfs") == 0);
        HEQ("K2_ASSETVFS_VERSION = 1", (long long)K2_ASSETVFS_VERSION, 1LL);
        HCHECK("K2_CHARLIB_IFACE string", strcmp(K2_CHARLIB_IFACE, "k2.characterlib") == 0);
        HEQ("K2_CHARLIB_VERSION = 1", (long long)K2_CHARLIB_VERSION, 1LL);

        HEQ("sizeof(K2AssetVfsApi) = 9 pointers", (long long)sizeof(K2AssetVfsApi), (long long)(9 * sizeof(void*)));
        HEQ("offsetof(K2AssetVfsApi, RegisterResManMissHandler)", (long long)offsetof(K2AssetVfsApi, RegisterResManMissHandler), (long long)(5 * sizeof(void*)));
        HEQ("offsetof(K2AssetVfsApi, UnregisterStreamFilter)", (long long)offsetof(K2AssetVfsApi, UnregisterStreamFilter), (long long)(8 * sizeof(void*)));
        HEQ("K2VFS_STREAM_PASS = 0", (long long)K2VFS_STREAM_PASS, 0LL);
        HEQ("K2VFS_STREAM_MEMORY = 1", (long long)K2VFS_STREAM_MEMORY, 1LL);
        HEQ("K2VFS_STREAM_REWRITE = 2", (long long)K2VFS_STREAM_REWRITE, 2LL);
        HEQ("sizeof(K2CharacterLibApi) = 6 pointers", (long long)sizeof(K2CharacterLibApi), (long long)(6 * sizeof(void*)));

        k2char::Catalog catalog;
        K2CharacterDefinition def = {};
        def.struct_size = sizeof(K2CharacterDefinition);
        def.character_id = "juhani";
        def.display_name = "Juhani";
        def.default_tag = "k1p_juhani";
        def.default_utc = "k1p_juhani";
        def.appearance_id = 818;
        def.head_id = 250;
        def.soundset_id = 70;

        HCHECK("registerDef succeeds", catalog.registerDef(&def, "test") == 1);
        HCHECK("has finds registered character", catalog.has("juhani") == 1);
        HCHECK("isAppearance finds appearance id", catalog.isAppearance(818) == 1);
        HEQ("isAppearance returns 0 for unregistered id", catalog.isAppearance(999), 0);
    }

    return kt::Report("common_contract_test");
}
